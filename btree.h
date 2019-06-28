#pragma once
// btree.h

#ifndef BTREE_H
#define BTREE_H

#include "btnode.h"
#include "recfile.h"
#include "fixfld.h"
#include "indbuff.h"
#include "stack.h"

// btree needs to be able to pack, unpack, read and
// 	write index records
// 	also, handle the insert, delete, split, merge,
//	growth of the tree
//	a pool of nodes will be managed
//	keep at least a branch of the tree in memory
//	
template <class keyType> class BTreeNode;
template <class keyType> class BTree
	// this is the full version of the BTree
{
protected:
	typedef BTreeNode<keyType> BTNode;// useful shorthand
	BTNode * FindLeaf(const keyType key);
	// load a branch into memory down to the leaf with key
	BTNode * NewNode();
	BTNode * Fetch(const int recaddr);
	int Store(BTNode *thisNode);
	BTNode Root;
	int Height; // height of tree
	int Order; // order of tree
	int PoolSize;
	BTNode ** Nodes; // pool of available nodes
					 // Nodes[1] is level 1, etc. (see FindLeaf)
					 // Nodes[Height-1] is leaf
	FixedFieldBuffer Buffer;
	RecordFile<BTNode> BTreeFile;
public:
	BTree(int order, int keySize = sizeof(keyType), int unique = 1);
	~BTree();
	int Open(char * name, int mode);
	int Create(char * name, int mode);
	int Close();
	int Insert(const keyType key, const int recAddr);
	int Remove(const keyType key, const int recAddr = -1);
	int Search(const keyType key, const int recAddr = -1);
	void InorderTraversal(ostream &);
	void Print(ostream &);
	void Print(ostream &, int nodeAddr, int level);

	void InorderPrint(ostream & stream, int nodeAddr, int level);

	void InOrderTraversal(ostream & stream);
	void InOrderTraversal(ostream & stream, int nodeAddr, int level);
};

const int MaxHeight = 5;
template <class keyType>
BTree<keyType>::BTree(int order, int keySize, int unique)
	: Buffer(1 + 2 * order, sizeof(int) + order * keySize + order * sizeof(int)),
	BTreeFile(Buffer), Root(order) {
	Height = 1;
	Order = order;
	PoolSize = MaxHeight * 2;
	Nodes = new BTNode *[PoolSize];
	BTNode::InitBuffer(Buffer, order);
	Nodes[0] = &Root;
}

template <class keyType>
BTree<keyType>::~BTree()
{
	Close();
	delete Nodes;
}

template <class keyType>
int BTree<keyType>::Open(char * name, int mode)
{
	int result;
	result = BTreeFile.Open(name, mode);
	if (!result) return result;
	// load root
	BTreeFile.Read(Root);
	Height = 1; // find height from BTreeFile!
	return 1;
}

template <class keyType>
int BTree<keyType>::Create(char * name, int mode) {
	int result;
	result = BTreeFile.Create(name, mode);
	if (!result) return result;
	// append root node
	result = BTreeFile.Write(Root);
	Root.RecAddr = result;
	return result != -1;
}

template <class keyType>
int BTree<keyType>::Close()
{
	int result;
	result = BTreeFile.Rewind();
	if (result) return result;
	result = BTreeFile.Write(Root);
	if (result == -1) return 0;
	return BTreeFile.Close();
}


template <class keyType>
int BTree<keyType>::Insert(const keyType key, const int recAddr)
{
	int result; int level = Height - 1;
	int newLargest = 0;
	keyType prevKey, largestKey;
	BTNode * thisNode = nullptr, *newNode = nullptr, *parentNode = nullptr;
	thisNode = FindLeaf(key);

	// test for special case of new largest key in tree
	// 탐색중인 노드에서 가장 큰 키값과 현재 키값을 비교하여 현재 키 값이 더 클 경우 최대 키 값을 교체함.
	if (key > thisNode->LargestKey())
	{
		newLargest = 1;
		prevKey = thisNode->LargestKey();
	}

	result = thisNode->Insert(key, recAddr);

	// handle special case of new largest key in tree
	// 위의 최대 키 값 조건을 만족했었다면 키 값을 업데이트함.
	if (newLargest)
		for (int i = 0; i < Height - 1; i++)
		{
			Nodes[i]->UpdateKey(prevKey, key);
			if (i > 0) Store(Nodes[i]);
		}

	while (result == -1) // if overflow and not root
	{
		//remember the largest key
		largestKey = thisNode->LargestKey();
		// split the node
		newNode = NewNode();
		thisNode->Split(newNode);
		Store(thisNode);
		Store(newNode);
		level--; // go up to parent level
		if (level < 0) break;
		// insert newNode into parent of thisNode
		parentNode = Nodes[level];
		result = parentNode->UpdateKey(largestKey, thisNode->LargestKey());
		result = parentNode->Insert(newNode->LargestKey(), newNode->RecAddr);
		thisNode = parentNode;
	}
	Store(thisNode);
	if (level >= 0) return 1;// insert complete
							 // else we just split the root
	int newAddr = BTreeFile.Append(Root); // put previous root into file
										  // insert 2 keys in new root node
	Root.Keys[0] = thisNode->LargestKey();
	Root.RecAddrs[0] = newAddr;
	Root.Keys[1] = newNode->LargestKey();
	Root.RecAddrs[1] = newNode->RecAddr;
	Root.NumKeys = 2;
	Height++;
	return 1;
}

template <class keyType>
int BTree<keyType>::Remove(const keyType key, const int recAddr)
{
	int result; int level = Height - 1;
	int newLargest = 0;
	keyType prevKey, largestKey;
	BTNode * thisNode = nullptr, *newNode = nullptr, *parentNode = nullptr;
	thisNode = FindLeaf(key);

	result = thisNode->Remove(key, recAddr);

	// 삭제 후 단말 노드의 record 갯수가 0개일 떄
	// 단말 노드 제거 후 부모 노드에 있는 단말 노드의 원소(단말 노드의 최대값)도 제거해야 함.
	if (thisNode->numKeys() <= 0)
	{
		parentNode = Nodes[--level];

		// 부모 노드에 있는 원소 제거
		result = parentNode->Remove(key, parentNode->Search(key));

		// 부모 노드 키 업데이트
		while (level-- > 0) {
			int newAddr = Store(parentNode);
			thisNode = parentNode;
			parentNode = Nodes[level];
			result = parentNode->UpdateKey(key, thisNode->LargestKey());
			result = parentNode->Insert(thisNode->LargestKey(), thisNode->RecAddr);
		}
	}

	else {
		int newAddr = Store(thisNode);
		parentNode = Nodes[--level];
		thisNode->RecAddr = newAddr;
		result = parentNode->UpdateKey(key, thisNode->LargestKey());
		result = parentNode->Insert(thisNode->LargestKey(), thisNode->RecAddr);

		while (level-- > 0) {
			int newAddr = Store(parentNode);
			thisNode = parentNode;
			parentNode = Nodes[level];
			result = parentNode->UpdateKey(key, thisNode->LargestKey());
			result = parentNode->Insert(thisNode->LargestKey(), thisNode->RecAddr);
		}
	}

	return 1;
}

template <class keyType>
int BTree<keyType>::Search(const keyType key, const int recAddr)
{
	BTNode * leafNode;
	leafNode = FindLeaf(key);
	return leafNode->Search(key, recAddr);
}

template<class keyType>
void BTree<keyType>::InorderTraversal(ostream & stream)
{
	stream << "BTree of height " << Height << " is " << endl;
	//Root.Print(stream);
	if (Height > 1)
		for (int i = 0; i < Root.numKeys(); i++)
		{
			InorderPrint(stream, Root.RecAddrs[i], 2);
		}
	stream << "end of BTree" << endl;
}

template <class keyType>
void BTree<keyType>::Print(ostream & stream)
{
	stream << "BTree of height " << Height << " is " << endl;
	Root.Print(stream);
	if (Height > 1)
		for (int i = 0; i < Root.numKeys(); i++)
		{
			Print(stream, Root.RecAddrs[i], 2);
		}
	stream << "end of BTree" << endl;
}

template <class keyType>
void BTree<keyType>::Print
(ostream & stream, int nodeAddr, int level)
{
	BTNode * thisNode = Fetch(nodeAddr);
	stream << "BTree::Print() ->Node at level " << level << " address " << nodeAddr << ' ' << endl;
	thisNode->Print(stream);
	if (Height > level)
	{
		level++;
		for (int i = 0; i < thisNode->numKeys(); i++)
		{
			Print(stream, thisNode->RecAddrs[i], level);
		}
		stream << "end of level " << level << endl;
	}
}

template <class keyType>
void BTree<keyType>::InorderPrint
(ostream & stream, int nodeAddr, int level)
{
	BTNode * thisNode = Fetch(nodeAddr);
	//stream << "BTree::Print() ->Node at level " << level << " address " << nodeAddr << ' ' << endl;
	if (Height > level)
	{
		level++;
		for (int i = 0; i < thisNode->numKeys(); i++)
		{
			InorderPrint(stream, thisNode->RecAddrs[i], level);
		}
		//stream << "end of level " << level << endl;
	}
	else
	{
		thisNode->Print(stream);
	}
}

template <class keyType>
BTreeNode<keyType> * BTree<keyType>::FindLeaf(const keyType key)
// load a branch into memory down to the leaf with key
{
	int recAddr, level;
	for (level = 1; level < Height; level++)
	{
		recAddr = Nodes[level - 1]->Search(key, -1, 0);//inexact search
		Nodes[level] = Fetch(recAddr);
	}
	return Nodes[level - 1];
}

template <class keyType>
BTreeNode<keyType> * BTree<keyType>::NewNode()
{// create a fresh node, insert into tree and set RecAddr member
	BTNode * newNode = new BTNode(Order);
	int recAddr = BTreeFile.Append(*newNode);
	newNode->RecAddr = recAddr;
	return newNode;
}

template <class keyType>
BTreeNode<keyType> * BTree<keyType>::Fetch(const int recaddr)
{// load this node from File into a new BTreeNode
	int result;
	BTNode * newNode = new BTNode(Order);
	result = BTreeFile.Read(*newNode, recaddr);
	if (result == -1) return NULL;
	newNode->RecAddr = result;
	return newNode;
}
template<class keyType>
int BTree<keyType>::Store(BTNode * thisNode)
{
	return BTreeFile.Write(*thisNode, thisNode->RecAddr);
}

//appended 19.5.29 for coding exam
template<class keyType>
void BTree<keyType>::InOrderTraversal(ostream & stream)
{
	stream << "BTree of height " << Height << " is " << endl;
	if (Height > 1)
		for (int i = 0; i < Root.numKeys(); i++)
		{
			InOrderTraversal(stream, Root.RecAddrs[i], 2);
		}
	stream << "end of BTree" << endl;
}

template<class keyType>
void BTree<keyType>::InOrderTraversal(ostream & stream, int nodeAddr, int level)
{
	BTNode * thisNode = Fetch(nodeAddr);
	//stream << "BTree::Print() ->Node at level " << level << " address " << nodeAddr << ' ' << endl;
	if (Height > level)
	{
		level++;
		for (int i = 0; i < thisNode->numKeys(); i++)
		{
			InOrderTraversal(stream, thisNode->RecAddrs[i], level);
		}
		stream << "end of level " << level << endl;
	}
	else
	{
		thisNode->Print(stream);
	}
}

#endif

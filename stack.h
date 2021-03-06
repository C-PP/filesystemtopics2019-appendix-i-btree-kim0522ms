#pragma once

template<typename T>
class tStack
{
private:
	typedef struct _Node
	{

		T Data;

		struct _Node* Next;



		_Node()

		{

			Data = NULL;

			Next = NULL;

		}



		_Node(T data)

		{

			Data = data;

			Next = NULL;

		}

	} NODE;



private:

	NODE *m_top, *m_bottom;



public:

	tStack()

	{

		m_top = new NODE();

		m_bottom = new NODE();



		Init();

	}



	~tStack()

	{

		Clear();



		delete m_top;

		delete m_bottom;



		m_top = NULL;

		m_bottom = NULL;

	}



	//스택 비우기

	void Clear(void)

	{

		NODE* indexNode = m_top->Next;

		for (indexNode; indexNode != m_bottom;)

		{

			NODE* deleteNode = indexNode;

			indexNode = indexNode->Next;



			delete deleteNode;

			deleteNode = NULL;

		}



		Init();

	}



	//입력

	void Push(T data)

	{

		//새 노드 생성 및 데이터 입력

		NODE *newNode = new NODE(data);



		//새 노드의 Next로 현재 제일 앞에 있는 노드를 지정

		newNode->Next = m_top->Next;



		//현재 노드를 제일 앞 노드로 지정

		m_top->Next = newNode;

	}



	//출력

	T Pop(void)

	{

		//첫 노드가 마지막 노드(m_bottom : 빈노드)면 데이터가 없음

		if (IsEmpty())

			return NULL;



		//첫번째 노드 데이터 수령

		NODE *deleteNode = m_top->Next;

		T result = deleteNode->Data;



		//2번째 노드를 첫번째 노드로

		m_top->Next = m_top->Next->Next;



		//첫번째 노드 삭제

		delete deleteNode;

		deleteNode = NULL;



		//결과 반환

		return result;

	}



	//비었는지 확인

	bool IsEmpty(void)

	{

		//첫 노드가 마지막 노드(m_bottom : 빈노드)면 데이터가 없음

		if (m_top->Next == m_bottom)

			return true;

		else

			return false;

	}



private:

	//초기화

	void Init(void)

	{

		m_top->Next = m_bottom;

		m_bottom->Next = m_bottom;

	}

};
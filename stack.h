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



	//���� ����

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



	//�Է�

	void Push(T data)

	{

		//�� ��� ���� �� ������ �Է�

		NODE *newNode = new NODE(data);



		//�� ����� Next�� ���� ���� �տ� �ִ� ��带 ����

		newNode->Next = m_top->Next;



		//���� ��带 ���� �� ���� ����

		m_top->Next = newNode;

	}



	//���

	T Pop(void)

	{

		//ù ��尡 ������ ���(m_bottom : ����)�� �����Ͱ� ����

		if (IsEmpty())

			return NULL;



		//ù��° ��� ������ ����

		NODE *deleteNode = m_top->Next;

		T result = deleteNode->Data;



		//2��° ��带 ù��° ����

		m_top->Next = m_top->Next->Next;



		//ù��° ��� ����

		delete deleteNode;

		deleteNode = NULL;



		//��� ��ȯ

		return result;

	}



	//������� Ȯ��

	bool IsEmpty(void)

	{

		//ù ��尡 ������ ���(m_bottom : ����)�� �����Ͱ� ����

		if (m_top->Next == m_bottom)

			return true;

		else

			return false;

	}



private:

	//�ʱ�ȭ

	void Init(void)

	{

		m_top->Next = m_bottom;

		m_bottom->Next = m_bottom;

	}

};
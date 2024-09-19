#pragma once

#define CRASH do { int* ptr = nullptr; *ptr = 0;} while(false)

#define BIT_NODEID 47 // 64 - 17
#define GET_NODEPTR(IDNode) ((DataNode<T>*)((unsigned long long)IDNode &  0x7FFFFFFFFFFF))
#define GET_NODEIDPTR(NodePtr, ID) (DataNode<T>*)((unsigned long long)NodePtr | ID << BIT_NODEID)

template <typename T>
class ObjectFreeList;

template <typename T>
struct DataNode
{
	ObjectFreeList<T>* underflowCheck;
	T data;
	ObjectFreeList<T>* overflowCheck;
	DataNode* next;
};

template <typename T>
class ObjectFreeList
{
public:
	ObjectFreeList<T>(bool placementNew);
	~ObjectFreeList<T>();

	T* Alloc();
	bool Free(T* obj);
	
private:
	DataNode<T>* _head;
	bool _bPlacementNew;

	/** 할당된 개수, 사용되고 있는 개수 체크 */
	long _capacity;
	long _useCount;

	long _NodeID;
};

template<typename T>
inline T* ObjectFreeList<T>::Alloc()
{
	DataNode<T>* newTop;
	DataNode<T>* retNode;
	do 
	{
		retNode = _head;
		if (retNode == nullptr)
		{
			_InterlockedIncrement(&_useCount);
			_InterlockedIncrement(&_capacity);

			DataNode<T>* newNode = new DataNode<T>();
			newNode->underflowCheck = this;
			newNode->overflowCheck = this;
			return &newNode->data;
		}
		newTop = GET_NODEPTR(retNode)->next;
	} while ((DataNode<T>*)_InterlockedCompareExchange64((long long*)&_head, (long long)newTop, (long long)retNode)
		!= retNode);

	_InterlockedIncrement(&_useCount);

	if (_bPlacementNew)
		new(&GET_NODEPTR(retNode)->data) T();

	return &GET_NODEPTR(retNode)->data;
}

template<typename T>
inline bool ObjectFreeList<T>::Free(T* obj)
{
	DataNode<T>* node = (DataNode<T>*)((unsigned long long)obj - sizeof(ObjectFreeList<T>*));

	if (node->underflowCheck != this)
		return false;
	if (node->overflowCheck != this)
		return false;

	/** 재할당할 시 생성자를 호출한다면 반환할 때 소멸자를 호출한다. */
	if (_bPlacementNew) obj->~T();
	

	DataNode<T>* t;
	do
	{
		t = _head;
		node->next = t;
	} while ((DataNode<T>*)_InterlockedCompareExchange64((long long*)&_head,
		(long long)GET_NODEIDPTR(node, (LONG64)_InterlockedIncrement((LONG*)&_NodeID)), (long long)t) != t);

	_InterlockedDecrement(&_useCount);

	return true;
}

template<typename T>
inline ObjectFreeList<T>::ObjectFreeList(bool placementNew) :
	_head(nullptr), _bPlacementNew(placementNew), _capacity(0), _useCount(0), _NodeID(0)
{

}

template<typename T>
inline ObjectFreeList<T>::~ObjectFreeList()
{
	if (_useCount > 0) CRASH;
	DataNode<T>* node = _head;
	for (int i = 0; i < _capacity; ++i)
	{
		DataNode<T>* nextNode = node->next;
		delete node;
		node = nextNode;
	}
	if (node != nullptr)
		CRASH;
}

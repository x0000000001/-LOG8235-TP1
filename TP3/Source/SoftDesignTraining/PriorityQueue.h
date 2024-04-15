#ifndef PRIORITYQUEUE_H
#define PRIORITYQUEUE_H

// Mostly taken from : https://gist.github.com/SomethingWithComputers/48e32b281d1bf8e662412e4113df43dc
// Added UpdatePriority() function to update the priority of all elements in the queue

template <typename InElementType>
struct TPriorityQueueNode {
    InElementType Element;
    float Priority;

    TPriorityQueueNode()
    {
    }

    TPriorityQueueNode(InElementType InElement, float InPriority)
    {
        Element = InElement;
        Priority = InPriority;
    }

    bool operator<(const TPriorityQueueNode<InElementType> Other) const
    {
        return Priority < Other.Priority;
    }
};

template <typename InElementType>
class TPriorityQueue {
public:
    TPriorityQueue()
    {
        Array.Heapify();
    }

public:
    // Always check if IsEmpty() before Pop-ing!
    InElementType Pop()
    {
        TPriorityQueueNode<InElementType> Node;
        Array.HeapPop(Node);
        return Node.Element;
    }

    TPriorityQueueNode<InElementType> PopNode()
    {
        TPriorityQueueNode<InElementType> Node;
        Array.HeapPop(Node);
        return Node;
    }

    void Push(InElementType Element, float Priority)
    {
        Array.HeapPush(TPriorityQueueNode<InElementType>(Element, Priority));
    }

    bool IsEmpty() const
    {
        return Array.Num() == 0;
    }

    void UpdatePriority()
    {
        for (TPriorityQueueNode<InElementType>& Node : Array)
        {
            Node.Priority = (Node.Priority == 0) ? 0 : Node.Priority - 1;
        }
        // Re-heapify the array once after updating priorities for all elements
        Array.Heapify();
    }

private:
    TArray<TPriorityQueueNode<InElementType>> Array;
};

#endif PRIORITYQUEUE_H
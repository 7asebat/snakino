#ifndef QUEUE_H_
#define QUEUE_H_

template <typename T, int S>
class Queue
{
public:
  /**
	 * The buffer capacity: read only as it cannot ever change.
	 */
  static constexpr int capacity = static_cast<int>(S);

  Queue();

  /**
	 * Adds an element to the beginning of buffer: the operation returns `false` if the addition caused overwriting an existing element.
	 */
  bool unshift(T value);

  /**
	 * Adds an element to the end of buffer: the operation returns `false` if the addition caused overwriting an existing element.
	 */
  bool push(T value);

  /**
	 * Removes an element from the beginning of the buffer.
	 * *WARNING* Calling this operation on an empty buffer has an unpredictable behaviour.
	 */
  T shift();

  /**
	 * Removes an element from the end of the buffer.
	 * *WARNING* Calling this operation on an empty buffer has an unpredictable behaviour.
	 */
  T pop();

  /**
	 * Returns the element at the beginning of the buffer.
	 */
  T first() const;

  /**
	 * Returns the element at the end of the buffer.
	 */
  T last() const;

  /**
	 * Array-like access to buffer.
	 * Calling this operation using and index value greater than `size - 1` returns the tail element.
	 * *WARNING* Calling this operation on an empty buffer has an unpredictable behaviour.
	 */
  T operator[](int index) const;

  /**
	 * Returns how many elements are actually stored in the buffer.
	 */
  int size() const;

  /**
	 * Returns how many elements can be safely pushed into the buffer.
	 */
  int available() const;

  /**
	 * Returns `true` if no elements can be removed from the buffer.
	 */
  bool isEmpty() const;

  /**
	 * Returns `true` if no elements can be added to the buffer without overwriting existing elements.
	 */
  bool isFull() const;

  /**
	 * Resets the buffer to a clean status, making all buffer positions available.
	 */
  void clear();

private:
  T buffer[S];
  T *head;
  T *tail;
  int count;
};

template <typename T, int S>
Queue<T, S>::Queue() : head(buffer), tail(buffer), count(0)
{
}

template <typename T, int S>
bool Queue<T, S>::unshift(T value)
{
  if (head == buffer)
  {
    head = buffer + capacity;
  }
  *--head = value;
  if (count == capacity)
  {
    if (tail-- == buffer)
    {
      tail = buffer + capacity - 1;
    }
    return false;
  }
  else
  {
    if (count++ == 0)
    {
      tail = head;
    }
    return true;
  }
}

template <typename T, int S>
bool Queue<T, S>::push(T value)
{
  if (++tail == buffer + capacity)
  {
    tail = buffer;
  }
  *tail = value;
  if (count == capacity)
  {
    if (++head == buffer + capacity)
    {
      head = buffer;
    }
    return false;
  }
  else
  {
    if (count++ == 0)
    {
      head = tail;
    }
    return true;
  }
}

template <typename T, int S>
T Queue<T, S>::shift()
{
  if (count == 0)
    return *head;
  T result = *head++;
  if (head >= buffer + capacity)
  {
    head = buffer;
  }
  count--;
  return result;
}

template <typename T, int S>
T Queue<T, S>::pop()
{
  if (count == 0)
    return *tail;
  T result = *tail--;
  if (tail < buffer)
  {
    tail = buffer + capacity - 1;
  }
  count--;
  return result;
}

template <typename T, int S>
T Queue<T, S>::first() const
{
  return *head;
}

template <typename T, int S>
T Queue<T, S>::last() const
{
  return *tail;
}

template <typename T, int S>
T Queue<T, S>::operator[](int index) const
{
  if (index >= count)
    return *tail;
  return *(buffer + ((head - buffer + index) % capacity));
}

template <typename T, int S>
int Queue<T, S>::size() const
{
  return count;
}

template <typename T, int S>
int Queue<T, S>::available() const
{
  return capacity - count;
}

template <typename T, int S>
bool Queue<T, S>::isEmpty() const
{
  return count == 0;
}

template <typename T, int S>
bool Queue<T, S>::isFull() const
{
  return count == capacity;
}

template <typename T, int S>
void Queue<T, S>::clear()
{
  head = tail = buffer;
  count = 0;
}

#endif

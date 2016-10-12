// Implementation of the templated Vector class
// ECE4122/6122 Lab 3
// Jeremy Feltracco

#include <iostream> // debugging
#include "Vector.h"

// Your implementation here
// Fill in all the necessary functions below
using namespace std;

// Default constructor
template <typename T>
Vector<T>::Vector() : elements(NULL), count(0), reserved(0)
{
}

// Copy constructor
template <typename T>
Vector<T>::Vector(const Vector& rhs)
{
    count = rhs.count;
    reserved = rhs.reserved;
    elements = (T*) malloc(reserved * sizeof(T));

    for (unsigned i = 0; i < count; ++i) {
        new (&elements[i]) T(rhs[i]);
    }
}

// Assignment operator
template <typename T>
Vector<T>& Vector<T>::operator=(const Vector& rhs)
{
    if (this != &rhs) {
        this->~Vector();
        count = rhs.Size();
        reserved = rhs.reserved;
        elements = (T*) malloc(reserved * sizeof(T));

        for (unsigned i = 0; i < count; ++i) {
            new (&elements[i]) T(rhs[i]);
        }
    }
    return *this;
}

// Destructor
template <typename T>
Vector<T>::~Vector()
{
    if (elements != NULL) {
        for (unsigned i = 0; i < count; ++i) {
            elements[i].~T();
        }
        free (elements);
    }
}

// Add and access front and back
template <typename T>
void Vector<T>::Push_Back(const T& rhs)
{
    if (count == reserved) {
        reserved = count + 1;
        T* new_arr = (T*) malloc(reserved * sizeof(T));

        for (unsigned i = 0; i < count; ++i) {
            new (&new_arr[i]) T(elements[i]);
        }

        this->~Vector();

        elements = new_arr;
    }

    new (&elements[count]) T(rhs);
    count++;
}

template <typename T>
void Vector<T>::Push_Front(const T& rhs)
{
    if (count == reserved) {
        reserved = count + 1;
        T* new_arr = (T*) malloc(reserved * sizeof(T));

        for (unsigned i = 0; i < count; ++i) {
            new (&new_arr[i + 1]) T(elements[i]);
        }

        if (elements != NULL) {
            for (unsigned i = 0; i < count; ++i) {
                elements[i].~T();
            }
            free (elements);
        }

        elements = new_arr;
    }

    new (&elements[0]) T(rhs);
    count++;
}

template <typename T>
void Vector<T>::Pop_Back()
{ // Remove last element
    elements[count - 1].~T();
    count--;
}

template <typename T>
void Vector<T>::Pop_Front()
{ // Remove first element
    elements[0].~T();

    for (unsigned i = 0; i < count - 1; ++i) {
        new (&elements[i]) T(elements[i + 1]);
        elements[i + 1].~T();
    }

    count--;
}

// Element Access
template <typename T>
T& Vector<T>::Front() const
{
    return elements[0];
}

// Element Access
template <typename T>
T& Vector<T>::Back() const
{
    return elements[count - 1];
}

template <typename T>
T& Vector<T>::operator[](size_t i) const
{
    return elements[i];
}

template <typename T>
T& Vector<T>::operator[](size_t i)
{
    return elements[i];
}

template <typename T>
size_t Vector<T>::Size() const
{
    return count;
}

template <typename T>
bool Vector<T>::Empty() const
{
    return count == 0;
}

// Implement clear
template <typename T>
void Vector<T>::Clear()
{
    this->~Vector();
    elements = NULL;
    count = 0;
    reserved = 0;
}

// Iterator access functions
template <typename T>
VectorIterator<T> Vector<T>::Begin() const
{
    return VectorIterator<T>(&elements[0]);
}

template <typename T>
VectorIterator<T> Vector<T>::End() const
{
    return VectorIterator<T>(&elements[count]);
}

// Implement the iterators

// Constructors
template <typename T>
VectorIterator<T>::VectorIterator()
{
    current = NULL;
}

template <typename T>
VectorIterator<T>::VectorIterator(T* c)
{
    current = c;
}

// Copy constructor
template <typename T>
VectorIterator<T>::VectorIterator(const VectorIterator<T>& rhs)
{
    current = rhs.current;
}

// Iterator defeferencing operator
template <typename T>
T& VectorIterator<T>::operator*() const
{
    return *current;
}

// Prefix increment
template <typename T>
VectorIterator<T>  VectorIterator<T>::operator++()
{
    current++;
    return VectorIterator(current);
}

// Postfix increment
template <typename T>
VectorIterator<T> VectorIterator<T>::operator++(int)
{
    VectorIterator orig = VectorIterator(current);
    current++;
    return orig;
}

// Comparison operators
template <typename T>
bool VectorIterator<T>::operator !=(const VectorIterator<T>& rhs) const
{
    return current != rhs.current;
}

template <typename T>
bool VectorIterator<T>::operator ==(const VectorIterator<T>& rhs) const
{
    return current == rhs.current;
}





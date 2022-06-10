#pragma once

#include <cstdint>
#include <stdexcept>
#include <iostream>

template <typename T>
class	SmartArray
{
	public:
		// Initializes an empty (but allocated) array
		SmartArray(void)
		{
			this->_array = new T[0];
			this->_capacity = 0;
		}

		// Initializes the array with the given size and default-initialized values
		SmartArray(std::size_t size)
		{
			this->_array = new T[size]();
			this->_capacity = size;
		}

		// Initializes the array with the given size value
		SmartArray(std::size_t size, T value)
		{
			this->_array = new T[size];
			this->_capacity = size;

			for (std::size_t i = 0; i < size; i++)
			{
				this->_array[i] = value;
			}
		}

		// Deallocates the array
		~SmartArray(void)
		{
			delete[] this->_array;
			this->_capacity = 0;
		}

		// Returns a reference to the element at the given index (performs bounds checking)
		T&	operator[](std::size_t index)
		{
			if (index >= this->_capacity)
			{
				throw std::out_of_range("Index out of range");
			}

			return (this->_array[index]);
		}

		// Returns a const reference to the element at the given index (performs bounds checking)
		const T&	operator[](std::size_t index) const
		{
			if (index >= this->_capacity)
			{
				throw std::out_of_range("Index out of range");
			}

			return (this->_array[index]);
		}

		// Deep copies 'other' into this array
		SmartArray&	operator=(const SmartArray& other)
		{
			if (this != &other)
			{
				delete[] this->_array;
				this->_array = new T[other._capacity];
				this->_capacity = other._capacity;

				for (std::size_t i = 0; i < this->_capacity; i++)
				{
					this->_array[i] = other._array[i];
				}
			}

			return (*this);
		}

		// Returns the capacity of the array
		std::size_t	getCapacity(void) const
		{
			return (this->_capacity);
		}

	private:
		T*			_array;
		std::size_t	_capacity;
};

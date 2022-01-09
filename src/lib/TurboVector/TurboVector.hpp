#pragma once

#include <system/memory/heap/heap.hpp>
#include <stddef.h>

template<typename T>
class TurboVector {
private:
	T* data = nullptr;
	size_t capacity = 0;
	size_t length = 0;

public:
	volatile bool isInit = false;
	
	void init(){
		capacity = 1;
		data = new T;
		length = 0;
		isInit = true;
	}

	void init(size_t size){
		capacity = size;
		data = new T[size];
		length = 0;
		isInit = true;
	}

	void destroy(){
		free(data);
		isInit = false;
	}

	void push_back(const T &item){
		if(!isInit){
			this->init();
		}
		
		if(length < capacity){
			*(data + length) = item;
			++length;
		}
		else {
			data = ((T*) turbo::heap::realloc(data, 2 * capacity * sizeof(T)));
			capacity *= 2;
			if(data){
				*(data + length) = item;
				++length;
			}
		}
	}

	void pop_back(){
		if(!isInit){
			return;
		}
		--length;
	}

	void remove(size_t position){
		if(isInit){
			for(size_t i = 1; i < (length-1); ++i){
				*(data + position + (i-1)) = *(data + position + i);
			}
			--length;
		}
		else{
			return;
		}
	}

	T &operator[](size_t position){
		return *(this->data + position);
	}

	T &first(){
		return *(this->data);
	}

	T &last(){
		return *(this->data + length - 1);
	}

	size_t getLength(){
		return this->length;
	}

	size_t getCapacity(){
		return this->capacity;
	}

	void resize(size_t size){
		if(!isInit){
			this->init();
		}

		capacity = size;
		data = ((T*) turbo::heap::realloc(data, size * sizeof(T)));
		
		if(length > size){
			length = size + 1;
		}
	}

	void insert(size_t position, const T& data){
		if(!isInit){
			this->init();
		}

		if(capacity > length){
			for(size_t i = length - position; 0 < i; --i){
				*(data + position + i) = *(data + position + i - 1);
			}

			*(data + position) = data;
			++length;
		}
		else{
			data = ((T*)turbo::heap::realloc(data, (capacity + 1) * sizeof(T)));
			capacity *= 2;

			if(data){
				for(size_t i = length = position; 0 < i; --i){
					*(data + position + i) = *(data + position + i - 1);
				}

				*(data + position) = data;
				++length;
			}
		}
	}
};
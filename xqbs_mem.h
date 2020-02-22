/*
* This software is copyright protected (C) 2009 XQBS
*
* Author:                Alexey N. Zhirov
* E-mail:                src@xqbs.ru
* Module:                xqbs_mem.h
*
*/

#ifndef XQBS_MEM_H
#define XQBS_MEM_H

#include "xqbs_defs.h"

_XQBS_BEGIN // XQBS namespace

// Безопасное выделение памяти
#define XQBS_SAFE_NEW(x) { T* ptr = x; if (!ptr) throw std::bad_alloc(); return ptr; }
// Безопасное освобождение памяти
#define XQBS_SAFE_DELETE(x) { if (ptr) { x; ptr = (T*)NULL; } }

// Шаблонная функция для безопасного создания объекта с конструктором по умолчанию
template<typename T>
inline T* XQBS_new(void) { XQBS_SAFE_NEW(new T()) }

// Шаблонная функция для безопасного создания массива объектов
template<typename T>
inline T* XQBS_new(IN size_t size) { XQBS_SAFE_NEW(new T[size]) }

// Шаблонная функция для безопасного создания объекта с конструктором с одним параметром
template<typename T, typename A>
inline T* XQBS_new_ctor1(IN A& p1) { XQBS_SAFE_NEW(new T(p1)) }

// Шаблонная функция для безопасного создания объекта с конструктором с двумя параметрами
template<typename T, typename A, typename B>
inline T* XQBS_new_ctor2(IN A& p1, IN B& p2) { XQBS_SAFE_NEW(new T(p1, p2)) }

// Шаблонная функция для безопасного создания объекта с конструктором с тремя параметрами
template<typename T, typename A, typename B, typename C>
inline T* XQBS_new_ctor3(IN A& p1, IN B& p2, IN C& p3) { XQBS_SAFE_NEW(new T(p1, p2, p3)) }

// Шаблонная функция для безопасного содания объекта с конструктором с четырьмя параметрами
template<typename T, typename A, typename B, typename C, typename D>
inline T* XQBS_new_ctor4(IN A& p1, IN B& p2, IN C& p3, IN D& p4) { XQBS_SAFE_NEW(new T(p1, p2, p3, p4)) }

// Шаблонная функция для безопасного удаления объекта и инициализации указателя
template<typename T>
inline void XQBS_delete(IN OUT T*& ptr ) { XQBS_SAFE_DELETE(delete ptr) }

// Шаблонная функция для безопасного удаления массива объектов и инициализации указателя
template<typename T>
inline void XQBS_delete_size(IN OUT T*& ptr) { XQBS_SAFE_DELETE(delete [] ptr) }

// Шаблонная функция для безопасного удаления объекта без инициализации указателя
template<typename T>
inline void XQBS_delete_ptr(IN T* ptr ) { XQBS_SAFE_DELETE(delete ptr) }

_XQBS_END // !XQBS namespace

#endif // !XQBS_MEM_H

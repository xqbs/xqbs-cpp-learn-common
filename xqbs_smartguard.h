/*
* This software is copyright protected (C) 2009 XQBS
*
* Author:                Alexey N. Zhirov
* E-mail:                src@xqbs.ru
* Module:                xqbs_smartguard.h
*
*/

#ifndef XQBS_SMART_GUARD_H
#define XQBS_SMART_GUARD_H

#include "xqbs_defs.h"
#include "xqbs_mem.h"
#include "xqbs_refbase.h"

_XQBS_BEGIN // XQBS namespace

// Мета-триггер для вызова глобальной функции
struct XQBS_GuardClassDefault{};
// Мета-триггер для вызова функции с параметром
struct XQBS_GuardWithParam{};
// Мета-триггер для вызова функции без параметра
struct XQBS_GuardWithoutParam{};

// Мнимый тип для определения функции удаления объектов через XQBS_SmartGuardBase::DeleteObject
struct XQBS_DeleteObjectFakeType{};
// Мнимый тип для определения функции удаления объектов через XQBS_SmartGuardBase::DeleteObjectArray
struct XQBS_DeleteObjectArrayFakeType{};


// Прототип функции для удаление объекта через XQBS_SmartGuardBase::DeleteObject
typedef XQBS_DeleteObjectFakeType* (*XQBS_DeleteObject)(void*);
// Прототип функции для удаление массива объектов через XQBS_SmartGuardBase::DeleteObjectArray
typedef XQBS_DeleteObjectArrayFakeType* (*XQBS_DeleteObjectArray)(void*);


///////////////////////////////////////////////////////////////////////////////
// Базовый класс SmartGuard-а с реализацией базовой функциональности
// P - это тип объекта (обычно это указатель на тип, например int* или std::string* и т.д.,
//     но может быть и не указатель, а например HANDLE)
// F - это прототип глобальной функции (например void(__cdecl*)(void*) - прототип функции free)
template< typename P, typename F >
class XQBS_SmartGuardBase
{
public:

    // Функция для удаления памяти выделенной через new
    static XQBS_DeleteObjectFakeType* DeleteObject(IN void* ptr) { XQBS_delete( reinterpret_cast<P&>(ptr) ); return NULL; }
    // Функция для удаления памяти выделенной через new []
    static XQBS_DeleteObjectArrayFakeType* DeleteObjectArray(IN void* ptr) { XQBS_delete_size( reinterpret_cast<P&>(ptr) ); return NULL; }

    // Конструктор для варианта, когда нельзя передать ссылку на указатель
    // p - это указатель на объект типа P;
    // f - это указатель на функцию, которую надо вызвать
    XQBS_SmartGuardBase(IN P p, IN F f) : m_d(p), m_p(m_d), m_f(f), m_v(NULL) {}

    // Конструктор для варианта, когда и функцию вызвать надо и указатель проинициализировать надо
    // p - это ссылка на указатель на объект;
    // f - это функция, которую надо вызвать
    // v - это значение которым надо проинициализировать указатель после вызова функции f
    XQBS_SmartGuardBase(IN OUT P& p, IN F f, IN P v) : m_p(p), m_f(f), m_v(v) {}

    // Оператор присваивания для инициализации SmartGuard-а другим указателем,
    // старый указатель перетирается БЕЗ вызова предзаданной функции!
    // p - это указатель, который необходимо поставить на контроль.
    //
    // ВНИМАНИЕ!!!
    //   1. При использовании данного оператора указатель p не инициализируется после вызова функции F
    //   2. При использовании данного оператора вызов функции F не делается!
    //   3. Для вызова функции F используйте явный вызов функции Reset() потомка.
    XQBS_SmartGuardBase& operator= (IN P p) { m_p = p; return *this; }

    // Для того, чтобы SmartGuard вел себя как указатель, определим неявное приведение к P&
    operator P& () { return m_p; }

    // Выполнить принудительный вызов функции F
    void Reset(void)
    {
        if (m_f && m_p != m_v)
        {
            m_f(m_p);
            m_p = m_v;
        }
    }

protected:

    // Виртуальный деструктор
    virtual ~XQBS_SmartGuardBase() {}

protected:

    P&   m_p; // Ссылка на указатель для обратной связи
    F    m_f; // Указатель на функцию
    P    m_v; // Значение для инициализации (по умолчанию NULL)
    P    m_d; // m_p - это всегда ссылка, поэтому для того, чтобы можно было инициализировать m_p
    // не только ссылкой на переменную, но и константой, используется этот финт ушами с m_d
};


///////////////////////////////////////////////////////////////////////////////
// Реализация SmartGuard-а для случая, когда надо вызывать функцию член класса
// P - указатель на тип
// F - прототип функции члена класса C, которую надо вызвать
// C - класс из которого надо вызвать функцию член,
//     имеет значение мета-триггера XQBS_GuardClassDefault, для того, чтобы можно было
//     вызывать частичную специализацию класса SmartGuard без указания С
// B - Мета-триггер для управления поведением SmartGuard-а, может принимать следующие значения:
//     XQBS_GuardWithParam    - для вызова функции F с параметром P (данное значение мета-триггера
//                              используется по умолчанию, т.к. чаще приходится вызывать функцию F
//                              с параметром P)
//     XQBS_GuardWithoutParam - для вызова функции F без параметра P
template< typename P, typename F, typename B = XQBS_GuardWithParam, typename C = XQBS_GuardClassDefault >
class XQBS_SmartGuard : public XQBS_SmartGuardBase<P, F>
{
protected:

    C&   m_c;  // Ссылка на объект класса C, используется для вызова функции члена f

protected:

    // Определение шаблона для управления поведением SmartGuard-а, определяемое мета-триггером B
    template<typename O, typename T> struct _Reset {};

    // Частичная спецификация шаблона для случая вызова функции F члена класса C с параметром P
    template<typename O> struct _Reset<O, XQBS_GuardWithParam>
    {
        // Конструктор с параметром, о - это объект класса C
        _Reset(IN OUT O& o)
        {
            if (o.m_f && o.m_p != o.m_v)
            {
                (o.m_c.*o.m_f)(o.m_p);
                o.m_p = o.m_v;
            }
        }
    };

    // Частичная спецификация шаблона для случая вызова функции F члена класса C без параметра
    template<typename O> struct _Reset<O, XQBS_GuardWithoutParam>
    {
        // Конструктор с параметром, о - это объект класса C
        _Reset(IN OUT O& o)
        {
            if (o.m_f && o.m_p != o.m_v)
            {
                (o.m_c.*o.m_f)();
                o.m_p = o.m_v;
            }
        }
    };

public:

    /// Конструктор для варианта, когда нельзя передать ссылку на указатель
    // p - это указатель на объект
    // f - это указатель на функцию член класса c
    // c - это объект класса C
    XQBS_SmartGuard(IN P p, IN F f, IN C& c) : XQBS_SmartGuardBase(p, f), m_c(c) {}
    // Конструктор для варианта, когда надо вызвать функцию f член класса c и
    // после этого проинициализировать указатель значением v
    // p - это указатель на объект
    // f - это указатель на функцию член класса C
    // v - это значение, которым надо проинициализировать p после вызова функции f
    // c - это объект класса C
    XQBS_SmartGuard(IN OUT P& p, IN F f, IN P v, IN C& c) : XQBS_SmartGuardBase(p, f, v), m_c(c) {}

    // Виртуальный деструктор с автоматическим вызовом функции Reset
    virtual ~XQBS_SmartGuard() {  Reset(); }

    // Выполнить принудительный вызов функции F с учетом значения мета-триггера B
    void Reset(void) { _Reset<XQBS_SmartGuard, B> R(*this); }
};


///////////////////////////////////////////////////////////////////////////////
// Частичная специализация SmartGuard-а для случая, когда надо вызывать глобальную функцию F
// P - это указатель на тип
// F - это прототип глобальной функции
template< typename P, typename F, typename B >
class XQBS_SmartGuard < P, F, B, XQBS_GuardClassDefault> : public XQBS_SmartGuardBase<P, F>
{
protected:

    // Определение шаблона для управления поведением SmartGuard-а, определяемое мета-триггером B
    template<typename O, typename T> struct _Reset {};

    // Частичная спецификация шаблона для случая вызова функции F члена класса C с параметром P
    template<typename O> struct _Reset<O, XQBS_GuardWithParam>
    {
        // Конструктор с параметром, о - это объект класса C
        _Reset(IN OUT O& o)
        {
            if (o.m_f && o.m_p != o.m_v)
            {
                o.m_f(o.m_p);
                o.m_p = o.m_v;
            }
        }
    };

    // Частичная спецификация шаблона для случая вызова функции F члена класса C без параметра
    template<typename O> struct _Reset<O, XQBS_GuardWithoutParam>
    {
        // Конструктор с параметром, о - это объект класса C
        _Reset(IN OUT O& o)
        {
            if (o.m_f && o.m_p != o.m_v)
            {
                o.m_f();
                o.m_p = o.m_v;
            }
        }
    };

public:

    // Конструктор для варианта, когда нельзя передать ссылку на указатель
    // p - это указатель на объект
    // f - это указатель на функцию, которую надо вызвать
    XQBS_SmartGuard(IN P p, IN F f) : XQBS_SmartGuardBase(p, f) {}
    // Конструктор для варианта, когда и функцию вызвать надо и указатель проинициализировать надо
    // p - это указатель на объект
    // f - это указатель на функцию, которую надо вызвать
    // v - это значение, которым надо проинициализировать p после вызова функции f
    XQBS_SmartGuard(IN OUT P& p, IN F f, IN P v) : XQBS_SmartGuardBase(p, f, v) {}

    // Виртуальный деструктор с автоматическим вызовом функции Reset
    virtual ~XQBS_SmartGuard() { Reset(); }

    // Выполнить принудительный вызов функции F с учетом значения мета-триггера B
    void Reset(void) { _Reset<XQBS_SmartGuard, B> R(*this); }
};


///////////////////////////////////////////////////////////////////////////////
// Частичная специализация SmartGuard-а для случая, когда надо вызывать DeleteObject
// P - это указатель на тип
template< typename P >
class XQBS_SmartGuard < P, XQBS_DeleteObject, XQBS_GuardWithParam, XQBS_GuardClassDefault > : public XQBS_SmartGuardBase<P, XQBS_DeleteObject>
{
public:

    /// Конструктор для варианта, когда нельзя передать ссылку на указатель
    // p - это указатель на объект
    XQBS_SmartGuard(IN P p) : XQBS_SmartGuardBase(p, XQBS_SmartGuardBase<P, XQBS_DeleteObject>::DeleteObject) {}
    // Конструктор для варианта, когда и функцию вызвать надо и указатель занулить надо
    // p - это указатель на объект
    // v - это значение, которым надо проинициализировать p после вызова DeleteObject
    XQBS_SmartGuard(IN OUT P& p, IN P v) : XQBS_SmartGuardBase(p, XQBS_SmartGuardBase<P, XQBS_DeleteObject>::DeleteObject, v) {}

    // Виртуальный деструктор с автоматическим вызовом функции Reset
    virtual ~XQBS_SmartGuard() { Reset();  }
};


///////////////////////////////////////////////////////////////////////////////
// Частичная специализация SmartGuard-а для случая, когда надо вызывать DeleteObjectArray
// P - это указатель на тип
template< typename P >
class XQBS_SmartGuard < P, XQBS_DeleteObjectArray, XQBS_GuardWithParam, XQBS_GuardClassDefault > : public XQBS_SmartGuardBase<P, XQBS_DeleteObjectArray>
{
public:

    // Конструктор для варианта, когда нельзя передать ссылку на указатель
    // p - это указатель на объект
    XQBS_SmartGuard(IN P p) : XQBS_SmartGuardBase(p, XQBS_SmartGuardBase<P, XQBS_DeleteObjectArray>::DeleteObjectArray) {}
    // Конструктор для варианта, когда и функцию вызвать надо и указатель инициализировать надо
    // p - это указатель на объект
    // v - это значение, которым надо проинициализировать p после вызова DeleteObject
    XQBS_SmartGuard(IN OUT P& p, IN P v) : XQBS_SmartGuardBase(p, XQBS_SmartGuardBase<P, XQBS_DeleteObjectArray>::DeleteObjectArray, v) {}

    // Виртуальный деструктор с автоматическим вызовом функции Reset
    virtual ~XQBS_SmartGuard() { Reset();  }
};


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
//// Определение наиболее часто используемых определений класса XQBS_SmartGuard ////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// Это класс удобен для хранения указателя на память выделенную функцией malloc,
// а также он обеспечивает автоматический вызов функции free.
template<typename P>
struct XQBS_SmartFree : public XQBS_SmartGuard<P, void(__cdecl*)(void*)>
{
    // Конструктор
    XQBS_SmartFree(IN P p) : XQBS_SmartGuard(p, ::free) {}
    // Конструктор
    XQBS_SmartFree(IN P& p, IN P v) : XQBS_SmartGuard(p, ::free, v) {}
};


///////////////////////////////////////////////////////////////////////////////
// Это класс удобен для хранения указателя на память выделенную оператором new,
// а также он обеспечивает автоматический вызов оператора delete.
template<typename P>
struct XQBS_SmartDelete : public XQBS_SmartGuard<P, XQBS_DeleteObject>
{
    // Конструктор
    XQBS_SmartDelete(IN P p) : XQBS_SmartGuard(p) {}
    // Конструктор
    XQBS_SmartDelete(IN P& p, IN P v) : XQBS_SmartGuard(p, v) {}
};


///////////////////////////////////////////////////////////////////////////////
// Это класс удобен для хранения указателя на память выделенную оператором new[],
// а также он обеспечивает автоматический вызов оператора delete[].
template<typename P>
struct XQBS_SmartDeleteArray : public XQBS_SmartGuard<P, XQBS_DeleteObjectArray>
{
    // Конструктор
    XQBS_SmartDeleteArray(IN P p) : XQBS_SmartGuard(p) {}
    // Конструктор
    XQBS_SmartDeleteArray(IN P& p, IN P v) : XQBS_SmartGuard(p, v) {}
};


///////////////////////////////////////////////////////////////////////////////
// Это класс удобен для хранения указателя на ссылочный объект, которому нельзя
// делать delete, поэтому он обеспечивает автоматический вызов функции Release()
template<typename P>
struct XQBS_SmartRelease : public XQBS_SmartGuard<P, LONG (XQBS_RefBase::*)(void), XQBS_GuardWithoutParam, XQBS_RefBase>
{
    // Конструктор
    XQBS_SmartRelease(IN P p) : XQBS_SmartGuard(p, &XQBS_RefBase::Release, *p) {}
    // Конструктор
    XQBS_SmartRelease(IN P& p, IN P v) : XQBS_SmartGuard(p, &XQBS_RefBase::Release, v, *p) {}
};


///////////////////////////////////////////////////////////////////////////////
// Это класс удобен для хранения дескриптора Windows, такие дескрипторы часто
// используются при программировании на Win32 API, поэтому данный класс
// обеспечивает автоматический вызов функции CloseHandle
struct XQBS_SmartHandle : public XQBS_SmartGuard<HANDLE, BOOL (__stdcall*)(HANDLE)>
{
    // Конструктор
    XQBS_SmartHandle(IN HANDLE h) : XQBS_SmartGuard(h, ::CloseHandle) {}
    // Конструктор
    XQBS_SmartHandle(IN HANDLE& h, IN HANDLE v) : XQBS_SmartGuard(h, ::CloseHandle, v) {}
};

_XQBS_END // !XQBS namespace

#endif // !XQBS_SMART_GUARD_H

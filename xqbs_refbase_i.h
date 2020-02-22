/*
 * This software is copyright protected (C) 2009 XQBS
 *
 * Author:                Alexey N. Zhirov
 * E-mail:                src@xqbs.ru
 * Module:                xqbs_refbase_i.h
 *
 */

#ifndef XQBS_REFBASE_I_H
#define XQBS_REFBASE_I_H

#include "xqbs_defs.h"
#include "xqbs_mem.h"

_XQBS_BEGIN // XQBS namespace

///////////////////////////////////////////////////////////////////////////////
// Многопоточная версия счетчика ссылок
class XQBS_RefBase_I
{

protected:

    // Деструктор
    virtual ~XQBS_RefBase_I() {}

public:

    // Конструктор
    XQBS_RefBase_I() {}

    // Удалить объект если количество ссылок на него равно нулю
    virtual LONG Release(void) = 0;
    // Добавить ссылку на объект
    virtual LONG AddRef(void) = 0;
};

// Шаблонная функция для безопасного удаления ссылки
template<typename T> inline void XQBS_release(IN OUT T*& ptr) { XQBS_SAFE_DELETE(ptr->Release()) }

// Вспомогательная шаблонная функция для удаления ссылки с объектов потомков класса XQBS_RefBase
template<typename T> inline void XQBS_destroy(IN XQBS_RefBase_I* fake,IN OUT T*& ptr ) { XQBS_release<T>(ptr); }
// Вспомогательная шаблонная функция для удаления объектов не наследников XQBS_RefBase
template<typename T> inline void XQBS_destroy(IN void* fake, IN OUT T*& ptr) { XQBS_delete<T>(ptr); }

// Шаблонная функция для боезопасного удаления объекта или удаления ссылки в зависимости от типа объекта
template<typename T> inline void XQBS_destroy(IN OUT T*& ptr) { XQBS_destroy<T>(ptr, ptr); }

_XQBS_END // !XQBS namespace

#endif // !XQBS_REFBASE_I_H

/*
* This software is copyright protected (C) 2009 XQBS
*
* Author:                Alexey N. Zhirov
* E-mail:                src@xqbs.ru
* Module:                xqbs_refbase.h
*
*/

#ifndef XQBS_REFBASE_H
#define XQBS_REFBASE_H

#include "xqbs_refbase_i.h"

_XQBS_BEGIN // XQBS namespace

///////////////////////////////////////////////////////////////////////////////
// Многопоточная версия счетчика ссылок
class XQBS_RefBase
{
    // Дружественная функция
    template<class T> friend inline void XQBS_delete_ptr( IN T* ptr );

private:

    // Счетчик ссылок
    volatile LONG m_RefCount;

protected:

    // Деструктор
    virtual ~XQBS_RefBase() {}

public:

    // Конструктор
    XQBS_RefBase() : m_RefCount(1) {}

    // Удалить объект если количество ссылок на него равно нулю
    virtual LONG Release(void)
    {
        // Уменьшаем количество ссылок на одну
        LONG RefCount = InterlockedDecrement(&m_RefCount);

        // Если количество ссылок на объект равно нулю, следовательно, пришло время сделать себе харакири
        if ( 0 == RefCount )
        {
            // Делаем себе харакири
            XQBS_delete_ptr(this);
        }

        // Возвращаем полученное значение счетчика
        return RefCount;
    }

    // Добавить ссылку на объект
    virtual LONG AddRef(void)
    {
        // Накидываем одну ссылку
        LONG RefCount = InterlockedIncrement(&m_RefCount);

        // Проверяем счетчик на переполнение
        if (0 >= RefCount)
            // В этом месте вы должны использовать свой класс исключений!
            throw std::exception("Invalid refcount value");

        // Возвращаем полученное значение счетчика
        return RefCount;
    }
};

_XQBS_END // !XQBS namespace

#endif // !XQBS_REFBASE_H

#pragma once
//-----------------------------------------------------------------------------
#include "StdAfx.h"
//-----------------------------------------------------------------------------
namespace MRContainer
{
    //! Извлечь и получить элемент из вектора по индексу
    //! \param Vector вектор
    //! \param Index индекс
    //! \return возвращает извлечённый элемент
    template <typename T> T VectorTakeAt(std::vector<T>& Vector, size_t Index)
    {
        T v = Vector[Index];

        auto it = Vector.begin();
        std::advance(it, Index);
        Vector.erase(it);

        return v;
    }

    //! Извлечь и получить элемент из вектора по значению
    //! \param vec вектор
    //! \param value значение
    //! \return возвращает извлечённый элемент
    template <typename T>
    std::optional<T> VectorTake(std::vector<T>& vec, T value)
    {
        auto it = std::find(vec.begin(), vec.end(), value);
        if (it != vec.end())
        {
            return std::make_optional(std::move(VectorTakeAt(vec, it - vec.begin())));
        }
        return std::nullopt;
    }

    //! Проверить наличие значения в векторе
    //! \param Vector вектор
    //! \param Value значение
    //! \return возвращает true еслит элемент присутствует, иначе - false
    template <typename T> bool VectorContains(const std::vector<T>& Vector, T Value)
    {
        return std::find(Vector.begin(), Vector.end(), Value) != Vector.end();
    }

    //! Удалить все указатели в векторе с занулением памяти
    //! \param v вектор
    template <typename T>
    void VectorDeletePtrs(std::vector<T>& v)
    {
        for (auto a : v)
        {
            delete a;
        }
        v.clear();
        v.shrink_to_fit();
    }
}
//-----------------------------------------------------------------------------

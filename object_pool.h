// object_pool.h: a simple & high-performance object pool implementation
#ifndef _OBJECT_POOL_H_
#define _OBJECT_POOL_H_

#include "politedef.h"
#include <assert.h>
#include <memory>
#include <map>

#define OBJECT_POOL_HEADER_ONLY

#if defined(OBJECT_POOL_HEADER_ONLY)
#define OBJECT_POOL_DECL inline
#else
#define OBJECT_POOL_DECL
#endif

#pragma warning(push)
#pragma warning(disable:4200)

namespace purelib {
namespace gc {

#define POOL_ESTIMATE_SIZE(element_type) sz_align(sizeof(element_type), sizeof(void*))

namespace detail {
    struct chunk_info
    {
        size_t element_size;
        size_t element_count;
    };
    inline bool operator <(const chunk_info& lhs, const chunk_info& rhs)
    {
        return lhs.element_size < rhs.element_size ||
            (lhs.element_size == rhs.element_size && lhs.element_count < rhs.element_count);
    }

    class object_pool
    {
        typedef struct free_link_node
        {
            free_link_node* next;
        } *free_link;

        typedef struct chunk_link_node
        {
            chunk_link_node* next;
            char data[0];
        } *chunk_link;

        object_pool(const object_pool&) = delete;
        void operator= (const object_pool&) = delete;

    public:
        OBJECT_POOL_DECL object_pool(size_t element_size, size_t element_count);

        OBJECT_POOL_DECL ~object_pool(void);

        OBJECT_POOL_DECL void purge(void);

        OBJECT_POOL_DECL void cleanup(void);

        OBJECT_POOL_DECL void* get(void);
        OBJECT_POOL_DECL void release(void* _Ptr);

    private:
        OBJECT_POOL_DECL void* allocate_from_chunk(void);
        OBJECT_POOL_DECL void* allocate_from_process_heap(void);
        
        OBJECT_POOL_DECL free_link_node* tidy_chunk(chunk_link chunk);

    private:
        free_link        free_link_; // link to free head
        chunk_link       chunk_; // chunk link
        const size_t     element_size_;
        const size_t     element_count_;

#if defined(_DEBUG)
        size_t           allocated_count_; // allocated count 
#endif
    };
    
#define DEFINE_OBJECT_POOL_ALLOCATION(ELEMENT_TYPE,ELEMENT_COUNT) \
public: \
    static void * operator new(size_t /*size*/) \
    { \
        return get_pool().get(); \
    } \
    \
    static void * operator new(size_t /*size*/, std::nothrow_t) \
    { \
        return get_pool().get(); \
    } \
    \
    static void operator delete(void *p) \
    { \
        get_pool().release(p); \
    } \
    \
    static purelib::gc::detail::object_pool& get_pool() \
    { \
        static purelib::gc::detail::object_pool s_pool(POOL_ESTIMATE_SIZE(ELEMENT_TYPE), ELEMENT_COUNT); \
        return s_pool; \
    }

#define DECLARE_OBJECT_POOL_ALLOCATION(ELEMENT_TYPE) \
public: \
    static void * operator new(size_t /*size*/); \
    static void * operator new(size_t /*size*/, std::nothrow_t); \
    static void operator delete(void *p); \
    static purelib::gc::detail::object_pool& get_pool();
    
#define IMPLEMENT_OBJECT_POOL_ALLOCATION(ELEMENT_TYPE,ELEMENT_COUNT) \
    void * ELEMENT_TYPE::operator new(size_t /*size*/) \
    { \
        return get_pool().get(); \
    } \
    \
    void * ELEMENT_TYPE::operator new(size_t /*size*/, std::nothrow_t) \
    { \
        return get_pool().get(); \
    } \
    \
    void ELEMENT_TYPE::operator delete(void *p) \
    { \
        get_pool().release(p); \
    } \
    \
    purelib::gc::detail::object_pool& ELEMENT_TYPE::get_pool() \
    { \
        static purelib::gc::detail::object_pool s_pool(POOL_ESTIMATE_SIZE(ELEMENT_TYPE), ELEMENT_COUNT); \
        return s_pool; \
    }
}; // namespace detail

template<typename _Ty, size_t _ElemCount = 512>
class object_pool : public detail::object_pool
{
    object_pool(const object_pool&) = delete;
    void operator= (const object_pool&) = delete;

public:
    object_pool(void) : detail::object_pool(POOL_ESTIMATE_SIZE(_Ty), _ElemCount)
    {
    }

    template<typename..._Args>
    _Ty* construct(const _Args&...args)
    {
        return new (get()) _Ty(args...);
    }

    void destroy(void* _Ptr)
    {

        ((_Ty*)_Ptr)->~_Ty(); // call the destructor
        release(_Ptr);
    }
};
    
}; // namespace: purelib::gc
}; // namespace: purelib

#if defined(OBJECT_POOL_HEADER_ONLY)
#include "object_pool.cpp"
#endif

#pragma warning(pop)

#endif
/*
* Copyright (c) 2012-2018 by HALX99,  ALL RIGHTS RESERVED.
* Consult your license regarding permissions and restrictions.
**/


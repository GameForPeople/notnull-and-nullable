/*
	Copyright 2023, Won Seong-Yeon. All Rights Reserved.
		KoreaGameMaker@gmail.com
		github.com/GameForPeople
*/

#pragma once


#ifndef WONSY_TYPEUTIL
#define WONSY_TYPEUTIL


#include <memory>
#include <functional>

namespace WonSY
{
	using InvalidType = void;

	template < typename T, typename = std::void_t<> >
	class IsRawPtr
		: public std::false_type 
	{
	public:
		using ElementType = InvalidType;
	};

	template < typename T >
	class IsRawPtr< T* >
		: public std::true_type 
	{
	public:
		using ElementType = std::remove_pointer< typename std::remove_cvref< T >::type >::type;
	};

	template < class T >
	concept RawPtrType = IsRawPtr< T >::value;
	
	template < typename T, typename = std::void_t<> >
	class IsSharedPtr
		: public std::false_type 
	{
	public:
		using ElementType = InvalidType;
	};
	
	template < typename T >
	//class IsSharedPtr< T, std::void_t< std::is_same< T, std::shared_ptr< typename std::remove_cvref< typename T::element_type >::type > > > >
	class IsSharedPtr< std::shared_ptr< T > >
		: public std::true_type 
	{
	public:
		using ElementType = std::shared_ptr< T >::element_type;
	};
	
	template < class T >
	concept SharedPtrType = IsSharedPtr< T >::value;
	
	template < typename T, typename = std::void_t<> >
	class IsUniquePtr
		: public std::false_type
	{
	public:
		using ElementType = InvalidType;
	};
	
	template < typename T >
	//class IsUniquePtr< T, std::void_t< std::is_same< T, std::unique_ptr< typename std::remove_cvref< typename T::element_type >::type > > > >
	class IsUniquePtr< std::unique_ptr< T > >
		: public std::true_type 
	{
	public:
		using ElementType = std::unique_ptr< T >::element_type;
	};
	
	template < class T >
	concept UniquePtrType = IsUniquePtr< T >::value;
	
	template < class T >
	class ElementType
	{
	public:
		using Type = typename std::conditional<
			IsRawPtr< T >::value, 
				typename IsRawPtr< T >::ElementType,
				typename std::conditional< IsSharedPtr< T >::value,
					typename IsSharedPtr< T >::ElementType,
					typename IsUniquePtr< T >::ElementType >::type >::type;
	};
}

template< class T >
using WsyElementType = WonSY::ElementType< T >;

template< class T >
using WsyRawPtr = T*;

template< class T >
using WsySharedPtr = std::shared_ptr< T >;

template< class T >
using WsyUniquePtr = std::unique_ptr< T >;

#endif

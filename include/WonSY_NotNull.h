/*
	Copyright 2023, Won Seong-Yeon. All Rights Reserved.
		KoreaGameMaker@gmail.com
		github.com/GameForPeople
*/

#pragma once


#ifndef WONSY_NOTNULL
#define WONSY_NOTNULL

#ifndef WONSY_MACRO
#define WONSY_MACRO

#define WONSY_FAIL_STATIC_ASSERT( msg )                     \
	[]< bool _ = false >() { static_assert( _, msg ); }();  

#endif

#include <memory>
#include <functional>

#include "WonSY_TypeUtil.h"


namespace WonSY
{
	/*
		"항상 Null이 아닌 Ptr의 개념"을 의미하는 NotNull로 Nullable에서 확장되거나, elementType으로 바로 만들 수 있습니다.
		 
		타입으로는, RawPointer와 SharedPtr, UniquePtr를 지원하며, 각 포인트의 특성을 그대로 활용합니다.

		[ Version ]
		// 0.1 
			- 기존의 C++ 표준의 포인터와 그 친구들로는 Null과 관련된 이슈에서 안전하기 어렵다고 판단하여, 이를 극복하기 위한 새로운 친구 제작

		// 0.2
			- NotNullRequire Concept 개념 적용( IsRawPtr< T >::value || IsSharedPtr< T >::value || IsUniquePtr< T >::value )
		
		// 0.3 
			- NotNull( const ElementType& dataElement )을 IsRawPtr< T >로 처리 시, 적절한 메모리 회수 인터페이스 제공이 어렵기 때문에,
			  RawPtr로 Notnull을 바로 만드는 케이스를 제한합니다.

		// 0.4 
			- T가 shared_ptr 일 경우, UseCount를 반환할 수 있도록 처리합니다.
			- ElementType가 인자로 전달되었을 때, enable_shared_from_this()의 상속 여부에 따라, shared_from_this() 처리하던 로직은 잘못된것임이 확인되어 제거합니다.
			- 별칭 WsyNotNullRaw, WsyNotNullShared, WsyNotNullUnique 추가하였습니다.

		// 0.5
			- ElementType&() operaotr 의 처리가 의도와 다르게 동작하지 않는 케이스를 확인해서, 그 경우의 명시적으로 멤버함수 "Data()"를 사용할 수 있도록 추가하였습니다.

		[ Known Issue ]
			- [ Ver 0.3에서 제한 ]IsRawPtr< T >로 NotNull로 바로 만든 케이스의 경우, 메모리를 해제해줄 방법이 없어, 메모리릭이 발생한다. 이와 관련되어 개선사항의 고려가 필요하다.
			- New가 실패하는 케이스에 대해서는 정상적으로 처리하지 못할 수 있습니다.
	*/

	template < typename T >
	concept NotNullRequire = IsRawPtr< T >::value || IsSharedPtr< T >::value || IsUniquePtr< T >::value;

	template< NotNullRequire _Type >
	class [[nodiscard]] NotNull
	{
	public:
		using Type        = _Type;
		using ElementType = WsyElementType< _Type >::Type;

	private:
		Type m_data;

	public:
#pragma region [ CTOR, DTOR ]
		// 기본 생성자는 제거합니다.
		NotNull() = delete;
		
		// 소멸자
		~NotNull() noexcept = default;
		// 주의 해야할 것은, Nullable은 메모리 관리를 위한 스마트 포인터가 아니므로, 
		// 소멸자에서 임의로 메모리와 관련된 처리를 해주지 않습니다.
		// 결국 Type 본연의 소멸 시 동작대로 처리될 것입니다.

		// 기존에 이미 NotNull< T >인 경우, 데이터가 항상 유효하기 때문에 복사해준다.
		NotNull( const NotNull< Type >& )                = default;
		NotNull& operator=( const NotNull< Type >& )     = default;
		NotNull( NotNull< Type >&& ) noexcept            = default;
		NotNull& operator=( NotNull< Type >&& ) noexcept = default;

	private:
		// 생성자를 private하게 처리하고 FactoryFunc( MY_NAME_IS_NOT_NULL__NICE_TO_MEET_YOU )로만 제공한다.
		NotNull( const Type& data )
			: m_data{ data }
		{
		}

		// 생성자를 private하게 처리하고 FactoryFunc( MakeNotNull )로만 제공한다.
		NotNull( const ElementType& dataElement )
			: m_data{ nullptr }
		{
			if constexpr ( WonSY::IsRawPtr< Type >::value )
			{
				// m_data = new ElementType( dataElement );
				WONSY_FAIL_STATIC_ASSERT( "Because there is no suitable interface for memory release, limit the case of creating NotNull < T*> directly with ElementType." );
			}
			else if constexpr ( WonSY::IsSharedPtr< Type >::value )
			{
				// [ ver 0.4 ] 으악;
				//if constexpr ( std::derived_from< ElementType, std::enable_shared_from_this< ElementType > > )
				//{
				//	m_data = dataElement.shared_from_this();
				//}
				//else
				//{
					m_data = std::make_shared< ElementType >( dataElement );
				//}
			}
			else if constexpr ( WonSY::IsUniquePtr< Type >::value )
			{
				m_data = std::make_unique< ElementType >( dataElement );
			}
			else
			{
				WONSY_FAIL_STATIC_ASSERT( "unSupported Type!" );
			}
		}
	public:
#pragma endregion

#pragma region [ FactoryFunc Or Othres ]
		[[nodiscard]] static NotNull< Type > MakeNotNull( const ElementType& ele )
		{
			return NotNull< Type >( ele );
		}

		// 매크로에 의해서만 실행되어야 하는 함수입니다.
		[[nodiscard]] static NotNull< Type > DoNotCall_____MY_NAME_IS_NOT_NULL__NICE_TO_MEET_YOU( const Type& data )
		{
			return NotNull< Type >( data );
		}

		// m_data를 반환하는 함수입니다.
		[[nodiscard]] Type DoNotCall_____WON_S_Y_BIRTHDAY_IS_JULY_3RD() const
		{
			return m_data;
		}
	public:
#pragma endregion

#pragma region [ Operator ]
		operator ElementType&() const noexcept
		{
			return *m_data;
		}

		// 위 Opertator이 의도와 다르게, 동작되지 않을 때 사용합니다.
		ElementType& operator()() const noexcept
		{
			return *m_data;
		}

		void operator=( const ElementType& ele )
		{
			*m_data = ele;
		}

		// operator DataElementType를 제공하기 때문에 operator*()를 제공하지 않습니다. 
		// 굳이 기존 포인터와 호환할 필요가 없어보이기. 때문인데, 기존 프로젝트의 적용 시에는 이에 맞게 적용이 필요합니다.
		// 
		//DataElementType& operator*() const noexcept
		//{
		//	return *m_data;
		//}

		// 포인터 자체를 외부로 반환할 경우, Delete 가능해지고 이에 따라 NotNull의 의미가 퇴색되기 떄문에 제한되었습니다.
		// notnull은 notnull이여야 합니다.
		//const DataElementType* const get() const noexcept
		//{
		//	if constexpr ( std::is_pointer< DataType >::value )
		//	{
		//		return m_data;
		//	}
		//	else 
		//	{
		//		return m_data.get();
		//	}
		//}
	public:
#pragma endregion

		long GetUseCount() const
		{
			if constexpr ( WonSY::IsSharedPtr< Type >::value )
			{
				return m_data.use_count();
			}
			else
			{
				WONSY_FAIL_STATIC_ASSERT( "unSupported Type!" );
			}
		}
	};
}

template< typename T >
using WsyNotNull = WonSY::NotNull< T >;

template< typename T >
using WsyNotNullRaw = WonSY::NotNull< WsyRawPtr< T > >;

template< typename T >
using WsyNotNullShared = WonSY::NotNull< WsySharedPtr< T > >;

template< typename T >
using WsyNotNullUnique = WonSY::NotNull< WsyUniquePtr< T > >;

#endif

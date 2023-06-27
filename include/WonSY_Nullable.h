/*
	Copyright 2023, Won Seong-Yeon. All Rights Reserved.
		KoreaGameMaker@gmail.com
		github.com/GameForPeople
*/

#pragma once


#ifndef WONSY_NULLABLE
#define WONSY_NULLABLE

#ifndef WONSY_MACRO
#define WONSY_MACRO

#define WONSY_FAIL_STATIC_ASSERT( msg )                     \
	[]< bool _ = false >() { static_assert( _, msg ); }();  

#endif

#include <memory>
#include <functional>

#include "WonSY_TypeUtil.h"
#include "WonSY_NotNull.h"

namespace WonSY
{
	/*
		"Null일 수도 있고 아닐수도 있는" 기존과 동일한 Ptr의 개념을 표현하지만, 
		"데이터의 접근 자체를 허용하지 않는 타입이기 때문에", Null참조를 발생시키지 않습니다.

		"데이터에 접근하기 위해서는 NotNull로의 변환( EXPAND_TO_NOTNULL ) 이후, 성공 시 접근이 가능합니다.

		[ Version ]
		// 0.1 
			- 기존의 C++ 표준의 포인터와 그 친구들로는 Null과 관련된 이슈에서 안전하기 어렵다고 판단하여, 
			  이를 극복하기 위한 새로운 친구 NotNull과 이와 Coupling되는 개념 Nullable를 Design합니다.

		// 0.2 
			- NullableRequire Concept 개념 적용( IsRawPtr< T >::value || IsSharedPtr< T >::value || IsUniquePtr< T >::value )

		// 0.3 
			- Nullable< UniquePtr< T > >의 NotNull로의 변환 시, NotNull< UniquePtr >에서 NotNull< SharedPtr >로 변환되도록 적용-개선합니다.
			
		// 0.4 
			- 변환 시 SharedPtr로 처리되면서( 0.3ver ) 원래 Nullable가 댕글링처리되는 이슈가 있어, NotNull은 Unique_ptr를 허용하고, Nullable은 unique_ptr을 재한하는 방향으로 진행한다.
			 매크로 이름 수정, 수정, 수정해서 결국 EXPAND_TO_NOTNULL로 적용합니다. ( 완전 마음에 들지는 않아; 추후 추가 수정이 될 수도;; )

		// 0.5
			- 불필요해보이는 생성자들 제거, 쓸데없이 복잡해보인다.
			- 메모리 해제 함수 명을, Clear에서, Release로 변경처리 하였습니다. ( T가 STL Container일 경우, container의 Clear함수와 착오할 수 있어, 표준과 중복되지않는 이름으로 변경합니다. 
			- EXPAND_TO_NOTNULL과 관련하여, 반복문에서 쓰일 매크로 추가하였습니다.

		// 0.6
			- ElementType가 인자로 전달되었을 때, enable_shared_from_this()의 상속 여부에 따라, shared_from_this() 처리하던 로직은 잘못된것임이 확인되어 제거합니다.
			- 별칭 WsyNullableRaw, WsyNullableShared 추가하였습니다.
	*/

	template < typename T >
	concept NullableRequire = IsRawPtr< T >::value || IsSharedPtr< T >::value; // 유니크 Ptr 이 미친새끼 아무리 생각해도, Nullable 하면 일터짐, Notnull만 되도록 한다 || IsUniquePtr< T >::value;

	template< NullableRequire _Type >
	class Nullable
	{
	public:
		using Type        = _Type;
		using ElementType = WsyElementType< _Type >::Type;

	private:
		Type m_data;

	public:
#pragma region [ CTOR, DTOR, Operator ]
		// 기본 생성자는 nullptr로 처리해준다.
		Nullable() noexcept
			: m_data{ nullptr }
		{
		}

		// 기존 data*를 얕게 복사한다. ( todo : 음, 스마트포인터면 이동을 고려한다. )
		Nullable( const Type data ) noexcept
			: m_data{ data }
		{
		}

		// 같은 nullable을 생성자 인자로 받을 수 있도록 처리해준다.
		Nullable( const Nullable< Type >& rhs ) = default;

		// NotNull에서 Nullable 전환은 언제든지 가능해야한다.
		Nullable( const NotNull< Type >& rhs )
			: m_data( rhs.DoNotCall_____WON_S_Y_BIRTHDAY_IS_JULY_3RD() )
		{
		}

		// nullptr을 인자로 받는 생성자를 추가해준다.
		Nullable( std::nullptr_t ) noexcept
			: m_data{ nullptr }
		{
		}

		// 같은 nullable을 인자로 받을 수 있도록 처리해준다. ( operator )
		Nullable& operator=( const Nullable< Type >& rhs ) = default;

		// NotNull에서 Nullable 전환은 언제든지 가능하도록 처리해준다. ( operator )
		Nullable& operator=( const NotNull< Type >& rhs )
		{
			m_data = rhs.DoNotCall_____WON_S_Y_BIRTHDAY_IS_JULY_3RD();
			return *this;
		}

		// Type에서 바로 할당이 될 수 있도록 처리해줍니다. ( operator )
		Nullable& operator=( const Type rhs )
		{
			m_data = rhs;
			return *this;
		}

		// nullptr  할당은 언제나 가능해야한다. 다만 Raw의 Onwer일 경우, 메모리 릭이 발생할 수 있다. ( operator )
		Nullable& operator=( std::nullptr_t )
		{
			m_data = nullptr;
			return *this;
		}

		// 소멸자
		~Nullable() noexcept = default;
		// 주의 해야할 것은, Nullable은 메모리 관리를 위한 스마트 포인터가 아니므로, 
		// 소멸자에서 임의로 메모리와 관련된 처리를 해주지 않습니다.
		// 결국 DataType 본연 타입의 소멸 시 동작대로 처리될 것입니다.

		// 널체크는 허용해줍니다.
		operator bool() const
		{
			return m_data != nullptr;
		}

	private:
		// public이였다가, private으로 처리하고, 전역함수를 통해서만 호출하도록 합니다.
		Nullable( const ElementType& dataElement ) noexcept
			: m_data{ nullptr }
		{
			if constexpr ( IsRawPtr< Type >::value )
			{
				m_data = new ElementType( dataElement );
			}
			else if constexpr ( IsSharedPtr< Type >::value )
			{
				// [ ver 0.6 ] 으악
				//if constexpr ( std::derived_from< ElementType, std::enable_shared_from_this< ElementType > > )
				//{
				//	m_data = dataElement.shared_from_this();
				//}
				//else
				//{
					m_data = std::make_shared< ElementType >( dataElement );
				//}
			}
			else
			{
				WONSY_FAIL_STATIC_ASSERT( "unSupported Type!" );
			}
		}

		//// 내부에서, m_data를 접근하기 위해 사용합니다.
		//const Type& _GetData() const noexcept
		//{
		//	return m_data;
		//}

	public:
#pragma endregion

	public:
		// Owner일 시, 호출해서 수동으로 메모리를 제거해줍니다.
		void Release()
		{
			if constexpr ( std::is_pointer< Type >::value )
			{
				delete m_data;
			}

			/* smart Pointer의 Data반환은 아래 nullptr 할당으로 처리됩니다. */
			m_data = nullptr;
		}

	public:
		[[nodiscard]] static Nullable< Type > MakeNullable( const ElementType& ele )
		{
			return Nullable< Type >( ele );
		}

		// 매크로에서만 사용할 함수, 이걸 다른데에서 직접 호출하는 경우가 없게 함수 이름 끔찍하게 작성한다.
		[[nodiscard]] NotNull< Type > DoNotCall_____HELLO_MY_NAME_IS_NULLABLE__WHAT_IS_YOUR_NAME() const
		{
			// 이 함수를 수동으로 호출하지 않고, 매크로에 의해 실행된다는 룰만 지켜진다면, 여기서는 m_data의 유효성이 인정된다.
			
			if constexpr ( IsRawPtr< Type >::value || IsSharedPtr< Type >::value ) 
				return NotNull< Type >::DoNotCall_____MY_NAME_IS_NOT_NULL__NICE_TO_MEET_YOU( m_data );
			// Ver 0.4
			// else if constexpr ( IsUniquePtr< Type >::value )
			// 	return NotNull< std::shared_ptr< ElementType > >::MY_NAME_IS_NOT_NULL__NICE_TO_MEET_YOU( m_data );
		}
	};
}

template< typename T >
using WsyNullable = WonSY::Nullable< T >;

template< typename T >
using WsyNullableRaw = WonSY::Nullable< WsyRawPtr< T > >;

template< typename T >
using WsyNullableShared = WonSY::Nullable< WsySharedPtr< T > >;

#define RETURN_VOID (void)(0)

#define __NOTNULL__TYPE__( X ) WonSY::NotNull< typename std::remove_pointer_t< typename std::remove_reference_t< typename std::remove_const_t< decltype( X ) > > >::Type >

#define EXPAND_TO_NOTNULL( nullableName, notNullName, If_Fail_ReturnValue ) \
	if ( !nullableName ) [[unlikely]]                        \
	{                                                        \
		return If_Fail_ReturnValue;                          \
	}                                                        \
	                                                         \
	__NOTNULL__TYPE__( nullableName ) notNullName = nullableName.DoNotCall_____HELLO_MY_NAME_IS_NULLABLE__WHAT_IS_YOUR_NAME();

#define EXPAND_TO_NOTNULL_EX( nullableName, notNullName, If_Fail_ReturnValue, If_Fail_Task )  \
	if ( !nullableName ) [[unlikely]]                                    \
	{                                                                    \
		If_Fail_Task                                                     \
		return If_Fail_ReturnValue;                                      \
	}                                                                    \
	                                                                     \
	__NOTNULL__TYPE__( nullableName ) notNullName = nullableName.DoNotCall_____HELLO_MY_NAME_IS_NULLABLE__WHAT_IS_YOUR_NAME();

#define EXPAND_TO_NOTNULL_CONTINUE( nullableName, notNullName ) \
	if ( !nullableName ) [[unlikely]]       \
	{                                       \
		continue;                           \
	}                                       \
	                                        \
	__NOTNULL__TYPE__( nullableName ) notNullName = nullableName.DoNotCall_____HELLO_MY_NAME_IS_NULLABLE__WHAT_IS_YOUR_NAME();

#define EXPAND_TO_NOTNULL_CONTINUE_EX( nullableName, notNullName, If_Fail_Task ) \
	if ( !nullableName ) [[unlikely]]       \
	{                                       \
		If_Fail_Task                        \
		continue;                           \
	}                                       \
	                                        \
	__NOTNULL__TYPE__( nullableName ) notNullName = nullableName.DoNotCall_____HELLO_MY_NAME_IS_NULLABLE__WHAT_IS_YOUR_NAME();

#endif

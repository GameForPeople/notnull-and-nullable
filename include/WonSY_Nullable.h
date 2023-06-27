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
		"Null�� ���� �ְ� �ƴҼ��� �ִ�" ������ ������ Ptr�� ������ ǥ��������, 
		"�������� ���� ��ü�� ������� �ʴ� Ÿ���̱� ������", Null������ �߻���Ű�� �ʽ��ϴ�.

		"�����Ϳ� �����ϱ� ���ؼ��� NotNull���� ��ȯ( EXPAND_TO_NOTNULL ) ����, ���� �� ������ �����մϴ�.

		[ Version ]
		// 0.1 
			- ������ C++ ǥ���� �����Ϳ� �� ģ����δ� Null�� ���õ� �̽����� �����ϱ� ��ƴٰ� �Ǵ��Ͽ�, 
			  �̸� �غ��ϱ� ���� ���ο� ģ�� NotNull�� �̿� Coupling�Ǵ� ���� Nullable�� Design�մϴ�.

		// 0.2 
			- NullableRequire Concept ���� ����( IsRawPtr< T >::value || IsSharedPtr< T >::value || IsUniquePtr< T >::value )

		// 0.3 
			- Nullable< UniquePtr< T > >�� NotNull���� ��ȯ ��, NotNull< UniquePtr >���� NotNull< SharedPtr >�� ��ȯ�ǵ��� ����-�����մϴ�.
			
		// 0.4 
			- ��ȯ �� SharedPtr�� ó���Ǹ鼭( 0.3ver ) ���� Nullable�� ��۸�ó���Ǵ� �̽��� �־�, NotNull�� Unique_ptr�� ����ϰ�, Nullable�� unique_ptr�� �����ϴ� �������� �����Ѵ�.
			 ��ũ�� �̸� ����, ����, �����ؼ� �ᱹ EXPAND_TO_NOTNULL�� �����մϴ�. ( ���� ������ ������ �ʾ�; ���� �߰� ������ �� ����;; )

		// 0.5
			- ���ʿ��غ��̴� �����ڵ� ����, �������� �����غ��δ�.
			- �޸� ���� �Լ� ����, Clear����, Release�� ����ó�� �Ͽ����ϴ�. ( T�� STL Container�� ���, container�� Clear�Լ��� ������ �� �־�, ǥ�ذ� �ߺ������ʴ� �̸����� �����մϴ�. 
			- EXPAND_TO_NOTNULL�� �����Ͽ�, �ݺ������� ���� ��ũ�� �߰��Ͽ����ϴ�.

		// 0.6
			- ElementType�� ���ڷ� ���޵Ǿ��� ��, enable_shared_from_this()�� ��� ���ο� ����, shared_from_this() ó���ϴ� ������ �߸��Ȱ����� Ȯ�εǾ� �����մϴ�.
			- ��Ī WsyNullableRaw, WsyNullableShared �߰��Ͽ����ϴ�.
	*/

	template < typename T >
	concept NullableRequire = IsRawPtr< T >::value || IsSharedPtr< T >::value; // ����ũ Ptr �� ��ģ���� �ƹ��� �����ص�, Nullable �ϸ� ������, Notnull�� �ǵ��� �Ѵ� || IsUniquePtr< T >::value;

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
		// �⺻ �����ڴ� nullptr�� ó�����ش�.
		Nullable() noexcept
			: m_data{ nullptr }
		{
		}

		// ���� data*�� ��� �����Ѵ�. ( todo : ��, ����Ʈ�����͸� �̵��� ����Ѵ�. )
		Nullable( const Type data ) noexcept
			: m_data{ data }
		{
		}

		// ���� nullable�� ������ ���ڷ� ���� �� �ֵ��� ó�����ش�.
		Nullable( const Nullable< Type >& rhs ) = default;

		// NotNull���� Nullable ��ȯ�� �������� �����ؾ��Ѵ�.
		Nullable( const NotNull< Type >& rhs )
			: m_data( rhs.DoNotCall_____WON_S_Y_BIRTHDAY_IS_JULY_3RD() )
		{
		}

		// nullptr�� ���ڷ� �޴� �����ڸ� �߰����ش�.
		Nullable( std::nullptr_t ) noexcept
			: m_data{ nullptr }
		{
		}

		// ���� nullable�� ���ڷ� ���� �� �ֵ��� ó�����ش�. ( operator )
		Nullable& operator=( const Nullable< Type >& rhs ) = default;

		// NotNull���� Nullable ��ȯ�� �������� �����ϵ��� ó�����ش�. ( operator )
		Nullable& operator=( const NotNull< Type >& rhs )
		{
			m_data = rhs.DoNotCall_____WON_S_Y_BIRTHDAY_IS_JULY_3RD();
			return *this;
		}

		// Type���� �ٷ� �Ҵ��� �� �� �ֵ��� ó�����ݴϴ�. ( operator )
		Nullable& operator=( const Type rhs )
		{
			m_data = rhs;
			return *this;
		}

		// nullptr  �Ҵ��� ������ �����ؾ��Ѵ�. �ٸ� Raw�� Onwer�� ���, �޸� ���� �߻��� �� �ִ�. ( operator )
		Nullable& operator=( std::nullptr_t )
		{
			m_data = nullptr;
			return *this;
		}

		// �Ҹ���
		~Nullable() noexcept = default;
		// ���� �ؾ��� ����, Nullable�� �޸� ������ ���� ����Ʈ �����Ͱ� �ƴϹǷ�, 
		// �Ҹ��ڿ��� ���Ƿ� �޸𸮿� ���õ� ó���� ������ �ʽ��ϴ�.
		// �ᱹ DataType ���� Ÿ���� �Ҹ� �� ���۴�� ó���� ���Դϴ�.

		// ��üũ�� ������ݴϴ�.
		operator bool() const
		{
			return m_data != nullptr;
		}

	private:
		// public�̿��ٰ�, private���� ó���ϰ�, �����Լ��� ���ؼ��� ȣ���ϵ��� �մϴ�.
		Nullable( const ElementType& dataElement ) noexcept
			: m_data{ nullptr }
		{
			if constexpr ( IsRawPtr< Type >::value )
			{
				m_data = new ElementType( dataElement );
			}
			else if constexpr ( IsSharedPtr< Type >::value )
			{
				// [ ver 0.6 ] ����
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

		//// ���ο���, m_data�� �����ϱ� ���� ����մϴ�.
		//const Type& _GetData() const noexcept
		//{
		//	return m_data;
		//}

	public:
#pragma endregion

	public:
		// Owner�� ��, ȣ���ؼ� �������� �޸𸮸� �������ݴϴ�.
		void Release()
		{
			if constexpr ( std::is_pointer< Type >::value )
			{
				delete m_data;
			}

			/* smart Pointer�� Data��ȯ�� �Ʒ� nullptr �Ҵ����� ó���˴ϴ�. */
			m_data = nullptr;
		}

	public:
		[[nodiscard]] static Nullable< Type > MakeNullable( const ElementType& ele )
		{
			return Nullable< Type >( ele );
		}

		// ��ũ�ο����� ����� �Լ�, �̰� �ٸ������� ���� ȣ���ϴ� ��찡 ���� �Լ� �̸� �����ϰ� �ۼ��Ѵ�.
		[[nodiscard]] NotNull< Type > DoNotCall_____HELLO_MY_NAME_IS_NULLABLE__WHAT_IS_YOUR_NAME() const
		{
			// �� �Լ��� �������� ȣ������ �ʰ�, ��ũ�ο� ���� ����ȴٴ� �길 �������ٸ�, ���⼭�� m_data�� ��ȿ���� �����ȴ�.
			
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

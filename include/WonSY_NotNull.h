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
		"�׻� Null�� �ƴ� Ptr�� ����"�� �ǹ��ϴ� NotNull�� Nullable���� Ȯ��ǰų�, elementType���� �ٷ� ���� �� �ֽ��ϴ�.
		 
		Ÿ�����δ�, RawPointer�� SharedPtr, UniquePtr�� �����ϸ�, �� ����Ʈ�� Ư���� �״�� Ȱ���մϴ�.

		[ Version ]
		// 0.1 
			- ������ C++ ǥ���� �����Ϳ� �� ģ����δ� Null�� ���õ� �̽����� �����ϱ� ��ƴٰ� �Ǵ��Ͽ�, �̸� �غ��ϱ� ���� ���ο� ģ�� ����

		// 0.2
			- NotNullRequire Concept ���� ����( IsRawPtr< T >::value || IsSharedPtr< T >::value || IsUniquePtr< T >::value )
		
		// 0.3 
			- NotNull( const ElementType& dataElement )�� IsRawPtr< T >�� ó�� ��, ������ �޸� ȸ�� �������̽� ������ ��Ʊ� ������,
			  RawPtr�� Notnull�� �ٷ� ����� ���̽��� �����մϴ�.

		// 0.4 
			- T�� shared_ptr �� ���, UseCount�� ��ȯ�� �� �ֵ��� ó���մϴ�.
			- ElementType�� ���ڷ� ���޵Ǿ��� ��, enable_shared_from_this()�� ��� ���ο� ����, shared_from_this() ó���ϴ� ������ �߸��Ȱ����� Ȯ�εǾ� �����մϴ�.
			- ��Ī WsyNotNullRaw, WsyNotNullShared, WsyNotNullUnique �߰��Ͽ����ϴ�.

		// 0.5
			- ElementType&() operaotr �� ó���� �ǵ��� �ٸ��� �������� �ʴ� ���̽��� Ȯ���ؼ�, �� ����� ��������� ����Լ� "Data()"�� ����� �� �ֵ��� �߰��Ͽ����ϴ�.

		[ Known Issue ]
			- [ Ver 0.3���� ���� ]IsRawPtr< T >�� NotNull�� �ٷ� ���� ���̽��� ���, �޸𸮸� �������� ����� ����, �޸𸮸��� �߻��Ѵ�. �̿� ���õǾ� ���������� ����� �ʿ��ϴ�.
			- New�� �����ϴ� ���̽��� ���ؼ��� ���������� ó������ ���� �� �ֽ��ϴ�.
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
		// �⺻ �����ڴ� �����մϴ�.
		NotNull() = delete;
		
		// �Ҹ���
		~NotNull() noexcept = default;
		// ���� �ؾ��� ����, Nullable�� �޸� ������ ���� ����Ʈ �����Ͱ� �ƴϹǷ�, 
		// �Ҹ��ڿ��� ���Ƿ� �޸𸮿� ���õ� ó���� ������ �ʽ��ϴ�.
		// �ᱹ Type ������ �Ҹ� �� ���۴�� ó���� ���Դϴ�.

		// ������ �̹� NotNull< T >�� ���, �����Ͱ� �׻� ��ȿ�ϱ� ������ �������ش�.
		NotNull( const NotNull< Type >& )                = default;
		NotNull& operator=( const NotNull< Type >& )     = default;
		NotNull( NotNull< Type >&& ) noexcept            = default;
		NotNull& operator=( NotNull< Type >&& ) noexcept = default;

	private:
		// �����ڸ� private�ϰ� ó���ϰ� FactoryFunc( MY_NAME_IS_NOT_NULL__NICE_TO_MEET_YOU )�θ� �����Ѵ�.
		NotNull( const Type& data )
			: m_data{ data }
		{
		}

		// �����ڸ� private�ϰ� ó���ϰ� FactoryFunc( MakeNotNull )�θ� �����Ѵ�.
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
				// [ ver 0.4 ] ����;
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

		// ��ũ�ο� ���ؼ��� ����Ǿ�� �ϴ� �Լ��Դϴ�.
		[[nodiscard]] static NotNull< Type > DoNotCall_____MY_NAME_IS_NOT_NULL__NICE_TO_MEET_YOU( const Type& data )
		{
			return NotNull< Type >( data );
		}

		// m_data�� ��ȯ�ϴ� �Լ��Դϴ�.
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

		// �� Opertator�� �ǵ��� �ٸ���, ���۵��� ���� �� ����մϴ�.
		ElementType& operator()() const noexcept
		{
			return *m_data;
		}

		void operator=( const ElementType& ele )
		{
			*m_data = ele;
		}

		// operator DataElementType�� �����ϱ� ������ operator*()�� �������� �ʽ��ϴ�. 
		// ���� ���� �����Ϳ� ȣȯ�� �ʿ䰡 ����̱�. �����ε�, ���� ������Ʈ�� ���� �ÿ��� �̿� �°� ������ �ʿ��մϴ�.
		// 
		//DataElementType& operator*() const noexcept
		//{
		//	return *m_data;
		//}

		// ������ ��ü�� �ܺη� ��ȯ�� ���, Delete ���������� �̿� ���� NotNull�� �ǹ̰� ����Ǳ� ������ ���ѵǾ����ϴ�.
		// notnull�� notnull�̿��� �մϴ�.
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

/*
	Copyright 2023, Won Seong-Yeon. All Rights Reserved.
		KoreaGameMaker@gmail.com
		github.com/GameForPeople
*/

#include <iostream>
#include <map>

#include "include/WonSY_NotNull.h"
#include "include/WonSY_Nullable.h"


int main()
{
	/*
		예제
			- C++20 이상에서 동작합니다.
		 
		기본적으로는 다음처럼 사용될 수 있습니다.

		- A. nullable 객체가 먼저 존재하고, 이후 데이터 접근 필요 시, notnull로 확장 후 성공 시 데이터에 접근하여 처리함.
		- B. notnull 객체를 바로 만들어 처리함.

		이를 통해, 기존 코드에서의 Null참조 이슈에서 충분히 안전해질 수 있습니다.

		
		추가적으로, 함수의 반환 값 혹은 인자에 이를 적절히 사용함으로서, 
			0. 불필요한 널참조들을 제거하여, 성능적인 이점을 가져올 수 있고,
			1. 타입을 바탕으로 함수의 의도를 더 자세하게 담아낼 수 있습니다.

		- C. 함수 반환 값에서의 활용
		- D. 함수 인자에서의 활용
	*/

	using ElementType               = int;
	const ElementType elementValue0 = 7;
	const ElementType elementValue1 = 3;

	/// A. nullable 객체와, nullable 객체를 통해 notnull 객체로 확장한 후, 데이터에 접근하는 기본 예제를 보여줍니다.
	{
		// 아래 주석을 수정하여 한 타입을 정의해주세요.
		// using _Type = ElementType*;
		using _Type = std::shared_ptr< ElementType >;
		// unique_ptr은 댕글링 이슈로 nullable 개체 생성이 제한됩니다.

		/// A0. nullable 객체 할당 및, 가능한 동작 소개
		{
			// nullable< _Type > 개체에 메모리를 할당합니다.
			WsyNullable< _Type > nullable = WsyNullable< _Type >::MakeNullable( elementValue0 );

			// Nullable 객체에서 지원되는 항목은 operator bool(), Release()밖에 없습니다. 유효하더라도 값을 확인할 수는 없습니다.
			if ( nullable )
				std::cout << "[A0 - 0] nullable is valid!" << std::endl;
			else
				std::cout << "[A0 - 0] nullable is nullptr!" << std::endl;

			// 메모리를 해제하고 nullable를 무효화합니다.
			nullable.Release();

			if ( nullable )
				std::cout << "[A0 - 1] nullable is valid!" << std::endl;
			else
				std::cout << "[A0 - 1] nullable is nullptr!" << std::endl;

			// Owner가 아닌 경우에는, 메모리를 해제하지 않고, nullable를 무효화합니다.
			_Type ownerPtr = [ & ] 
				{
					if constexpr ( std::is_same< _Type, ElementType* >::value )
						return new ElementType( elementValue0 );
					else
						return std::make_shared< ElementType >( elementValue0 );
				}();

			{
				nullable = ownerPtr;

				/* do something ~ */

				// 이 부분에서, _Type이 RawPtr이고, nullable를 Release를 호출할 경우, ownerPtr는 댕글링된다.
				nullable = nullptr;
			}

			//if constexpr ( std::is_same< _Type, ElementType* >::value )
			//{
			//	delete ownerPtr;
			//}
			//else
			//{
			//}
		}

		/// A1. nullable에서 notnull객체로 확장한 후, 데이터에 접근하는 예제 소개
		[ & ]() ->int
		{
			{
				WsyNullable< _Type > nullable = WsyNullable< _Type >::MakeNullable( elementValue0 );

				// nullable을 데이터에 접근하기 위해서는, notnull로 확장한 후 성공했을 경우에만 가능합니다.

				// nullable이 null일 경우 3번째 인자값으로 리턴하고, 아닐 경우 2번쨰 인자값을 변수명으로하는 WsyNotNull을 생성합니다.
				EXPAND_TO_NOTNULL( nullable, notnull, 0 );

				// 이제 메모리에 접근하여 읽고 쓸수 있습니다. 이렇게 생성된 notnull 객체는 댕글링 되지 않는 이상 항상 notnull의 유효성이 유지됩니다.
				std::cout << "[A1 - 0] notnull Value is " << notnull << std::endl;

				notnull = elementValue1;

				std::cout << "[A1 - 1] notnull Value is " << notnull << std::endl;

				// ! 물론 이상태에서, 
				//	0. _Type이 RawPointer이고, nullable을 Release해버리면,        notnull은 댕글링됩니다.
				//	1. _Type이 RawPointer이고, nullable에 nullptr를 할당해버리면, 메모리릭이 발생합니다.
			}

			{
				WsyNullable< _Type > nullable = nullptr;

				// nullable이 유효하지 안호을 경우, 리턴합니다.
				EXPAND_TO_NOTNULL( nullable, notnull, 0 );

				// 따라서 아래 출력문은 실제 출력되지 않습니다.
				std::cout << "[A1 - 2] not call " << notnull << std::endl;
			}

			return 0;
		}();
	}

	/// B. notnull 객체를 바로 만들어 처리합니다.
	{
		// 아래 주석을 수정하여 한 타입을 정의해주세요.
		// using _Type = ElementType*;
		using _Type = std::shared_ptr< ElementType >;
		// using _Type = std::unique_ptr< ElementType >;

		// 어떠한 ElementType이 제공되면, 항상 할당된 상태가 가능합니다.
		WsyNotNull< _Type > notnull = WsyNotNull< _Type >::MakeNotNull( elementValue0 );

		// notnull 객체이므로, 바로 메모리에 접근하여 읽고 쓸 수 있습니다.
		std::cout << "[B0 - 0] notnull Value is " << notnull << std::endl;

		// notnull 객체에서, notnull 객체로의 복사 등은 당연히 항상 성공합니다.
		WsyNotNull< _Type > copiedNotnull = notnull;
		copiedNotnull = notnull;

		std::cout << "[B0 - 1] notnull copied Value is " << copiedNotnull << std::endl;
		copiedNotnull = elementValue1;
		std::cout << "[B0 - 2] notnull Value is " << notnull << std::endl;

		// notnull 객체에서, nullable 객체로의 변환 또한 당연히 항상 성공합니다.
		WsyNullable< _Type > nullable = notnull;
		nullable = notnull;
	}

	/// C. 함수 반환 값에서의 활용
	{
		std::map< int, WsyNotNull< std::shared_ptr< int > > > cont;
		const int defaultValue = 7;

		// 예를 들어 어떠한 컨테이너가 있고, 이 컨테이너와 관련하여 2개의 함수를 만들어야 되는 상황을 가정합니다.
		//	0. Get( x )     : 컨테이너에 인자로 받은 요소가 있으면, 해당 요소를 반환하고, 없으면 nullptr를 반환해준다.
		//	1. Acquire( x ) : 컨테이너에 인자로 받은 요소가 있으면, 해당 요소를 반환하고, 없으면 만들어서 반환해준다. 
		
		const auto Get     = [ &cont ]( const int keyValue ) -> WsyNullable< std::shared_ptr< int > >
			{
				if ( 
					const auto findResult = cont.find( keyValue );
					findResult != cont.end() )
					return findResult->second;

				return nullptr;
			};
		const auto Acquire = [ &cont, defaultValue /*, &Get */ ]( const int keyValue ) -> WsyNotNull< std::shared_ptr< int > >
			{
				// 안타깝게도, Get을 사용해서 처리한 것이 올바르지만, 적절하게 처리가 애매하다;
				//if ( 
				//	const auto getResult = Get( keyValue );
				//	getResult )
				//{
				//}

				// 결국 복붙; 뿅;
				if ( 
					const auto findResult = cont.find( keyValue );
					findResult != cont.end() )
					return findResult->second;

				auto notNull = WsyNotNull< std::shared_ptr< int > >::MakeNotNull( defaultValue );
				cont.insert( { keyValue, notNull } );

				return notNull;
			};

		// Get으로 받은 경우, Nullable이므로, NotNull로 확장해야 접근 가능하다.
		[ & ]()
		{
			auto nullable = Get( defaultValue );

			EXPAND_TO_NOTNULL( nullable, notnull, RETURN_VOID );

			// Get의 결과가 유효하지 않을 것이므로, 이 부분은 호출되지 않습니다.
			std::cout << "[C0 - 0] notnull Value is " << notnull << std::endl;
		}();

		// Acquire으로 받은 경우, NotNull이므로, 별도의 처리없이 접근이 가능합니다.
		auto notnull = Acquire( defaultValue );
		std::cout << "[C0 - 1] notnull Value is " << notnull << std::endl;

		// 결과적으로 함수 반환 값에 notnull과 nullable을 사용함으로서, 함수의 의도를 더 온전히 담을 수 있습니다.
	}

	/// D. 함수 인자에서의 활용
	{
		/// D0. 함수 의도에 대한 고민
		{
			// 아래와 같이 작성된 기존 함수가 있을 때, 다음과 같은 고민을 하게됩니다.
			const auto DoSomething0 = [ & ]( int* /* ptr */ )
				{
					/* 
					[ 함수의 정의를 확인하거나 수정해야할 때 으악]
					
					 아니 여기서, ptr이 null이면 리턴을 해야하는 건지; 
					 ptr이 null이면, ptr에 접근하는 부분만 뺴고는  동작시켜줘야하는건지;

					 명확하게 if( !ptr ) return 가 없다면, 의외로 명확하지 않은 경우가 많다 으악..

						// 보통 C++에서는 포인터와 레퍼런스를 사용해 이를 구분하고는 하지만, 
						// 레퍼런스를 넣는 과정에서 널참조 이슈가 발생하는 경우가 있어 완벽한 해결책으로 보기 어려움.
					 */

					// Do Something...
				};

			DoSomething0( nullptr );
			/* 
				[ 함수를 호출해야할 때 으악]
				
				- 음? 이런 파라미터 여기서 없는디 nullptr넣어두 잘 동작하려나?
			 */


			// Nullable와 NotNull를 적절히 사용할 경우, 그 의도를 명확하게 할 수 있습니다.
			{
				// 아 이 함수는, param이 null일때도, 동작하도록 작성되어야 하는구나!
				const auto DoSomethingWithNullable = [ & ]( const WsyNullable< std::shared_ptr< int > >& param )
					{

						// Do Something...

						if ( param )
						{
							EXPAND_TO_NOTNULL( param, notnull, RETURN_VOID );

							// Do Something...
						}
						else
						{
							// Do Something...
						}

						// Do Something...
					};

				// 이 경우, 유효하든, 유효하지 않든 모두 호출이 가능하다.
				DoSomethingWithNullable( nullptr );
				DoSomethingWithNullable( WsyNullable< std::shared_ptr< int > >::MakeNullable( 7 ) );
				DoSomethingWithNullable( WsyNotNull< std::shared_ptr< int > >::MakeNotNull( 7 ) );



				// 아 이 함수는, 항상 유효한 Param를 필요로 하는 함수구나
				const auto DoSomethingWithNotNull = [ & ]( const WsyNotNull< std::shared_ptr< int > >& /* param */ )
					{
					};
			
				// 이 경우, 유효한 인자가 있을 경우에만 호출이 가능하다.
				// DoSomethingWithNotNull( nullptr );
				// DoSomethingWithNotNull( WsyNullable< std::shared_ptr< int > >::MakeNullable( 7 ) );
				DoSomethingWithNotNull( WsyNotNull< std::shared_ptr< int > >::MakeNotNull( 7 ) );
			}
		}

		// 이를 통해, 한번 Null 체크된 인자를 다시 널체크하지 않을 수 있는 장점이 있습니다.
		{
			// old
			[]( int* ptr )
			{
				if ( !ptr )
					return;

				[]( int* ptr )
				{
					// 으악
					if ( !ptr )
						return;
					
					[]( int* ptr )
					{
						// 으악
						if ( !ptr )
							return;

					}( ptr );
				}( ptr );
			}( new int( 7 ) );

			// notnull
			[]( const WsyNullable< int* >& ptr )
			{
				EXPAND_TO_NOTNULL( ptr, notnullPtr, RETURN_VOID );

				[]( const WsyNotNull< int* >& notnullPtr )
				{
					[]( const WsyNotNull< int* >& notnullPtr )
					{
						// it's ok
						[]( const WsyNullable< int* >& /* ptr */ )
						{
						}( notnullPtr );
					}( notnullPtr );
				}( notnullPtr );
			}( WsyNullable< int* >::MakeNullable( 7 ) );
		}
	}

	// 예제는 여기까지입니다. 제 코드를 봐주셔서 감사합니다.

	return 0;
}
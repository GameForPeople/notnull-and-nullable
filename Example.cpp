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
		nullable< T > : null일 수도 있고, null이 아닐 수도 있는 상태의 개체로, null여부와 상관 없이 메모리 접근이 불가능합니다.
		notnull< T >  : null이 절대 아닌( 메모리 할당이 실패하는 케이스를 제외하고는 )개체로, nullable에서 확장된 개체이거나, 데이터로 생성된 개체입니다.
		
		이래는 예제 코드입니다. ( C++20 이상에서 동작합니다. )
		
		{
			기본적으로는 다음의 쓰임으로 사용됩니다.
			- [예제A] nullable 객체가 있을 때, 데이터 접근이 필요 시, nullable객체를 notnull객체로 확장을 시도한 이후, 성공 시 데이터에 접근하여 처리합니다.
			- [예제B] notnull 객체를 바로 만들어서 처리합니다.

			이를 통해, 기존 코드에서의 Null참조 이슈에서 충분히 안전해질 수 있습니다.
		}
		
		{
			추가적으로, 함수의 반환 값 혹은 인자에 이를 적절히 사용함으로서, 
				0. 불필요한 널참조들을 제거하여, 성능적인 이점을 가져올 수 있고,
				1. 타입을 바탕으로 함수의 의도를 더 자세하게 담아낼 수 있습니다.

			- [예제C] 함수 반환 값에서의 적용 예시입니다.
			- [예제D] 함수 인자에서의 적용 예시입니다.
		}

		{
			- [예제E] 이외의 경우들에 대하여 공유합니다.
		}
	*/

	using ElementType               = int;
	const ElementType elementValue0 = 7;
	const ElementType elementValue1 = 3;

	/// [예제A] nullable 개체의 생성, nullable 의 멤버 함수, nullable 개체를 통해 notnull 개체로의 확장과 데이터 접근하는 기본 예제 코드입니다.
	{
		// 아래 주석을 수정하여 테스트하실 타입을 결정해주세요. ( unique_ptr은 댕글링 이슈로 nullable 개체 생성이 제한됩니다. )
		// using _Type = ElementType*;
		using _Type = std::shared_ptr< ElementType >;

		/// 예제A0. nullable 개체의 생성 및 멤버 함수
		{
			// nullable 개체에 메모리를 할당합니다.
			WsyNullable< _Type > nullable = WsyNullable< _Type >::MakeNullable( elementValue0 );

			// Nullable 클래스의 Public 항목은 operator bool(), Release()밖에 없습니다. 개체가 유효한( !null )인 것을 알더라도 메모리에 접근( 값을 확인 )할 수 없습니다.
			if ( nullable )
				std::cout << "[A0 - 0] nullable is valid!" << std::endl;
			else
				std::cout << "[A0 - 0] nullable is nullptr!" << std::endl;

			// 메모리를 해제하고 nullable 개체를 무효화합니다. ( raw Pointer일 경우 delete를 호출하고, shared_ptr일 경우, 현재 문맥의 shared_ptr를 무효화합니다. ) 
			nullable.Release();

			if ( nullable )
				std::cout << "[A0 - 1] nullable is valid!" << std::endl;
			else
				std::cout << "[A0 - 1] nullable is nullptr!" << std::endl;

			// 소유권을 가진 Owner가 아닐 겨웅에는, Release를 할 경우, 문제가 될 수 있습니다.
			_Type ownerPtr = [ & ] 
				{
					if constexpr ( std::is_same< _Type, ElementType* >::value )
						return new ElementType( elementValue0 );
					else
						return std::make_shared< ElementType >( elementValue0 );
				}();

			// Owner가 아닌 경우에는, 메모리를 해제하지 않고, nullable를 무효화해야합니다.
			{
				nullable = ownerPtr;

				/* do something ~ */

				// 이 부분에서, _Type이 RawPtr이고, nullable의 Release()를 호출할 경우, ownerPtr는 댕글링되기 때문에, 단순 nullptr 대입을 해주어야 합니다.
				nullable = nullptr;
			}

			// ownerPtr가 raw Ptr일 때, 메모리 릭이 발생합니다.
		}

		/// 예제A1. nullable에서 notnull객체로 확장한 후, 데이터에 접근하는 예제 코드입니다.
		[ & ]() ->int
		{
			{
				WsyNullable< _Type > nullable = WsyNullable< _Type >::MakeNullable( elementValue0 );

				// nullable을 데이터에 접근하기 위해서는, notnull로 확장한 후 성공했을 경우에만 가능합니다.

				// EXPAND_TO_NOTNULL 매크로는, 
				// 1번째 인자값으로 들어온 nullable 개체가
				//	- null일 경우 3번째 인자값으로 리턴하고
				//	- null이 아닐 경우 2번쨰 인자값을 변수명으로 하는 WsyNotNull을 생성합니다.
				EXPAND_TO_NOTNULL( nullable, notnull, 0 );

				// 이제 notnull 개체가 생성되어, 메모리에 접근하여 읽고 쓸 수 있습니다. 
				// 이렇게 생성된 notnull 객체는 댕글링 되지 않는 이상 항상 notnull의 유효성이 유지됩니다.
				std::cout << "[A1 - 0] notnull Value is " << notnull << std::endl;
				notnull = elementValue1;

				std::cout << "[A1 - 1] notnull Value is " << notnull << std::endl;

				// ! 이 상태에서, 
				//	0. _Type이 RawPointer이고, 확장한 nullable을 Release해버리면,        원본 notnull은 댕글링됩니다.
				//	1. _Type이 RawPointer이고, 확장한 nullable에 nullptr를 할당해버리고, 원본 nullable을 Release하지 않으면, 메모리릭이 발생합니다.
			}

			{
				WsyNullable< _Type > nullable = nullptr;

				// nullable이 유효하지 않을 경우에는, notnull 개체를 생성하지 못하고 리턴합니다.
				EXPAND_TO_NOTNULL( nullable, notnull, 0 );

				// 따라서 아래 출력문은 실제 출력되지 않습니다.
				std::cout << "[A1 - 2] not call " << notnull << std::endl;
			}

			// EXPAND_TO_NOTNULL외에도, EXPAND_TO_NOTNULL_CONTINUE 등의 확장 매크로 기능이 존재합니다.

			return 0;
		}();
	}

	/// [예제B] notnull 객체를 바로 만들어 처리합니다.
	{
		// 아래 주석을 수정하여 테스트하실 타입을 결정해주세요.
		// using _Type = ElementType*;
		using _Type = std::shared_ptr< ElementType >;
		// using _Type = std::unique_ptr< ElementType >;

		// 어떠한 ElementType이 제공되면, 해당 값으로 할당하기 때문에, 항상 할당된 상태가 가능합니다. 
		// ( 다만 RawPtr 타입은 메모리 릭이 발생할 수 있어, MakeNotNull을 통한 생성을 제한합니다. )
		WsyNotNull< _Type > notnull = WsyNotNull< _Type >::MakeNotNull( elementValue0 );

		// notnull 객체이므로, 바로 메모리에 접근하여 읽고 쓸 수 있습니다.
		std::cout << "[B0 - 0] notnull Value is " << notnull << std::endl;

		// notnull 객체에서, notnull 객체로의 복사 등은 당연히 항상 성공하므로 허용합니다.
		WsyNotNull< _Type > copiedNotnull = notnull;
		copiedNotnull = notnull;

		std::cout << "[B0 - 1] notnull copied Value is " << copiedNotnull << std::endl;
		copiedNotnull = elementValue1;
		std::cout << "[B0 - 2] notnull Value is " << notnull << std::endl;

		// notnull 객체에서, nullable 객체로의 변환 또한 당연히 항상 성공하므로 허용합니다.
		WsyNullable< _Type > nullable = notnull;
		nullable = notnull;
	}

	/// [예제C] 함수 반환 값에서의 활용 예제입니다.
	{
		std::map< int, WsyNotNull< std::shared_ptr< int > > > cont;
		const int defaultValue = 7;
		const int invalidValue = 4;

		// 예를 들어 어떠한 컨테이너가 있고, 이 컨테이너와 관련하여 2개의 함수를 만들어야 되는 상황을 가정합니다.
		//	0. Get( x )     : 컨테이너에 인자로 받은 요소가 있으면, 해당 요소를 반환하고, 없으면 nullptr를 반환해준다.
		//	1. Acquire( x ) : 컨테이너에 인자로 받은 요소가 있으면, 해당 요소를 반환하고, 없으면 만들어서 반환해준다. 
		
		const auto Get = [ &cont ]( const int keyValue ) -> WsyNullable< std::shared_ptr< int > >
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

		// Get의 반환값의 경우 Nullable이므로, NotNull로 확장해야 접근 가능합니다.
		[ & ]()
		{
			auto nullable = Get( invalidValue );

			EXPAND_TO_NOTNULL( nullable, notnull, RETURN_VOID );

			// Get의 결과가 유효하지 않을 것이므로, 이 부분은 호출되지 않습니다.
			std::cout << "[C0 - 0] notnull Value is " << notnull << std::endl;
		}();

		// Acquire의 반환값의 경우 NotNull이므로, 별도의 처리없이 접근이 가능합니다.
		auto notnull = Acquire( invalidValue );
		std::cout << "[C0 - 1] notnull Value is " << notnull << std::endl;

		// 결과적으로 함수 반환 값에 notnull과 nullable을 사용함으로서, 함수의 의도를 더 온전히 담을 수 있습니다.
	}

	/// [예제D] 함수 인자에서의 활용 예제입니다.
	{
		/// D0. 함수 의도에 대한 고민
		{
			// 아래와 같이 작성된 기존 함수가 있을 때, 다음과 같은 고민을 하게됩니다.
			
			// [ 함수의 정의를 확인하거나 수정해야할 때의 고민 ]
			const auto DoSomething0 = [ & ]( int* /* ptr */ )
				{
					/* 
					 아니 여기서, ptr이 null이면 리턴을 해야하는 건지; 
					 ptr이 null이면, ptr에 접근하는 부분만 뺴고는  동작시켜줘야하는건지;

					 명확하게 if( !ptr ) return 가 없다면, 의외로 명확하지 않은 경우가 많습니다.

						// 보통 C++에서는 포인터와 레퍼런스를 사용해 이를 구분하고는 하지만, 
						// 레퍼런스를 넣는 과정에서 널참조 이슈가 발생하는 경우가 있어 완벽한 해결책으로 보기 어렵습니다.
					 */
				};

			// [ 함수를 호출해야할 때의 고민 ]
			DoSomething0( nullptr );
			/* 
				- 음? 인자가 int* 인데, 적당한 파라미터 여기서 없는디 nullptr넣어두 잘 동작하려나?
			 */

			// 결과적으로 Nullable와 NotNull를 적절히 사용할 경우에는, 그 의도를 명확하게 할 수 있습니다.
			{
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

					// 이 경우, 유효하든, 유효하지 않든 모두 호출이 가능합니다.
					DoSomethingWithNullable( nullptr );
					DoSomethingWithNullable( WsyNullable< std::shared_ptr< int > >::MakeNullable( 7 ) );
					DoSomethingWithNullable( WsyNotNull< std::shared_ptr< int > >::MakeNotNull( 7 ) );
				}

				{
					// 아 이 함수는, 항상 유효한 Param를 필요로 하는 함수구나
					const auto DoSomethingWithNotNull = [ & ]( const WsyNotNull< std::shared_ptr< int > >& param )
						{
							param;
						};
			
					// 이 경우, 유효한 인자를 제공해야지만 호출이 가능합니다.
					// DoSomethingWithNotNull( nullptr );
					// DoSomethingWithNotNull( WsyNullable< std::shared_ptr< int > >::MakeNullable( 7 ) );
					DoSomethingWithNotNull( WsyNotNull< std::shared_ptr< int > >::MakeNotNull( 7 ) );
				}
			}
		}

		// 이를 통해, 한번 Null 체크된 개체를, 다시 널 체크하지 않을 수 있는 장점이 있습니다.
		{
			// 기존 포인터 방식 시,
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

			// notnull 사용 시,
			[]( const WsyNullable< int* >& ptr )
			{
				EXPAND_TO_NOTNULL( ptr, notnullPtr, RETURN_VOID );

				[]( const WsyNotNull< int* >& notnullPtr )
				{
					[]( const WsyNotNull< int* >& notnullPtr )
					{
						// 중간에 nullable 개체로의 변환도 가능합니다.
						[]( const WsyNullable< int* >& nullablePtr )
						{
							nullablePtr;
						}( notnullPtr );
					}( notnullPtr );
				}( notnullPtr );
			}( WsyNullable< int* >::MakeNullable( 7 ) );
		}
	}

	/// [예제E] 이외의 경우들에 대하여 공유합니다.
	{
		/// E0. SharedPtr의 레퍼런스 카운트 처리
		{
			/// 기본 Shared_ptr처럼 Nullable, NotNull은  생성 및 소멸 시, ref Count는 증가 감소합니다.
			{
				WsySharedPtr< int > sharedPtr = std::make_shared< int >( 7 );

				// use_count 1
				std::cout << "[E0 - 0] shared_ptr Use Count : " << sharedPtr.use_count() << std::endl;

				{
					WsyNullable< WsySharedPtr< int > > nullable = sharedPtr;

					// use_count 2
					std::cout << "[E0 - 1] shared_ptr Use Count : " << sharedPtr.use_count() << std::endl;

					{
						EXPAND_TO_NOTNULL( nullable, notnull, 0 );

						// use_count 3
						std::cout << "[E0 - 2] shared_ptr Use Count : " << sharedPtr.use_count() << std::endl;

						auto copiedNotnull = static_cast<decltype(notnull)>(notnull);

						// use_count 4
						std::cout << "[E0 - 3] shared_ptr Use Count : " << sharedPtr.use_count() << std::endl;

						[[maybe_unused]] const auto& refedNotnull = notnull;

						// use_count 4
						std::cout << "[E0 - 4] shared_ptr Use Count : " << sharedPtr.use_count() << std::endl;
					}

					// use_count 2
					std::cout << "[E0 - 5] shared_ptr Use Count : " << sharedPtr.use_count() << std::endl;
				}

				// use_count 1
				std::cout << "[E0 - 6] shared_ptr Use Count : " << sharedPtr.use_count() << std::endl;
			}
		}
	}

	// 감사합니다.
	return 0;
}
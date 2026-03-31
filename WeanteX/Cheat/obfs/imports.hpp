


#ifndef LAZY_IMPORTER_HPP
#define LAZY_IMPORTER_HPP

#include "pe.hh"

#define LAZY_IMPORTER_HARDENED_MODULE_CHECKS
#define LAZY_IMPORTER_CACHE_OPERATOR_PARENS

#define IMPORT(name) ::li::detail::lazy_function<LAZY_IMPORTER_KHASH(#name), decltype(&name)>()

#define IMPORT_DEF(name) ::li::detail::lazy_function<LAZY_IMPORTER_KHASH(#name), name>()

#define IMPORT_MODULE(name) ::li::detail::lazy_module<LAZY_IMPORTER_KHASH(name)>()

#ifndef LAZY_IMPORTER_CPP_FORWARD
#ifdef LAZY_IMPORTER_NO_CPP_FORWARD
#define LAZY_IMPORTER_CPP_FORWARD(t, v) v
#else
#include <utility>
#define LAZY_IMPORTER_CPP_FORWARD(t, v) std::forward<t>( v )
#endif
#endif

#ifndef LAZY_IMPORTER_NO_FORCEINLINE
#if defined(_MSC_VER)
#define LAZY_IMPORTER_FORCEINLINE __forceinline
#elif defined(__GNUC__) && __GNUC__ > 3
#define LAZY_IMPORTER_FORCEINLINE inline __attribute__((__always_inline__))
#else
#define LAZY_IMPORTER_FORCEINLINE inline
#endif
#else
#define LAZY_IMPORTER_FORCEINLINE inline
#endif


#ifdef LAZY_IMPORTER_CASE_INSENSITIVE
#define LAZY_IMPORTER_CASE_SENSITIVITY false
#else
#define LAZY_IMPORTER_CASE_SENSITIVITY true
#endif

#define LAZY_IMPORTER_STRINGIZE(x) #x
#define LAZY_IMPORTER_STRINGIZE_EXPAND(x) LAZY_IMPORTER_STRINGIZE(x)

#define LAZY_IMPORTER_KHASH(str) ::li::detail::khash(str, \
    ::li::detail::khash_impl( __TIME__ __DATE__ LAZY_IMPORTER_STRINGIZE_EXPAND(__LINE__) LAZY_IMPORTER_STRINGIZE_EXPAND(__COUNTER__), 2166136261 ))

namespace li {
	namespace detail {
		struct forwarded_hashes {
			unsigned module_hash;
			unsigned function_hash;
		};


		using offset_hash_pair = unsigned long long;

		LAZY_IMPORTER_FORCEINLINE constexpr unsigned get_hash( offset_hash_pair pair ) noexcept { return ( pair & 0xFFFFFFFF ); }

		LAZY_IMPORTER_FORCEINLINE constexpr unsigned get_offset( offset_hash_pair pair ) noexcept { return static_cast< unsigned >( pair >> 32 ); }

		template<bool CaseSensitive = LAZY_IMPORTER_CASE_SENSITIVITY>
		LAZY_IMPORTER_FORCEINLINE constexpr unsigned hash_single( unsigned value, char c ) noexcept {
			return ( value ^ static_cast< unsigned >( ( !CaseSensitive && c >= 'A' && c <= 'Z' ) ? ( c | ( 1 << 5 ) ) : c ) ) * 16777619;
		}

		LAZY_IMPORTER_FORCEINLINE constexpr unsigned
			khash_impl( const char* str, unsigned value ) noexcept {
			return ( *str ? khash_impl( str + 1, hash_single( value, *str ) ) : value );
		}

		LAZY_IMPORTER_FORCEINLINE constexpr offset_hash_pair khash(
			const char* str, unsigned offset ) noexcept {
			return ( ( offset_hash_pair{ offset } << 32 ) | khash_impl( str, offset ) );
		}

		template<class CharT = char>
		LAZY_IMPORTER_FORCEINLINE unsigned hash( const CharT* str, unsigned offset ) noexcept {
			unsigned value = offset;

			for ( ;;) {
				char c = *str++;
				if ( !c )
					return value;
				value = hash_single( value, c );
			}
		}

		LAZY_IMPORTER_FORCEINLINE unsigned hash(
			const pe::UNICODE_STRING& str, unsigned offset ) noexcept {
			auto       first = str.m_buffer;
			const auto last = first + ( str.m_length / sizeof( wchar_t ) );
			auto       value = offset;
			for ( ; first != last; ++first )
				value = hash_single( value, static_cast< char >( *first ) );

			return value;
		}

		LAZY_IMPORTER_FORCEINLINE forwarded_hashes hash_forwarded(
			const char* str, unsigned offset ) noexcept {
			forwarded_hashes res{ offset, offset };

			for ( ; *str != '.'; ++str )
				res.module_hash = hash_single<true>( res.module_hash, *str );

			++str;

			for ( ; *str; ++str )
				res.function_hash = hash_single( res.function_hash, *str );

			return res;
		}


		LAZY_IMPORTER_FORCEINLINE const pe::PEB* peb( ) noexcept {
#if defined(_M_X64) || defined(__amd64__)
#if defined(_MSC_VER)
			return reinterpret_cast< const pe::PEB* >( __readgsqword( 0x60 ) );
#else
			const pe::PEB* ptr;
			__asm__ __volatile__( "mov %%gs:0x60, %0" : "=r"( ptr ) );
			return ptr;
#endif
#elif defined(_M_IX86) || defined(__i386__)
#if defined(_MSC_VER)
			return reinterpret_cast< const pe::PEB* >( __readfsdword( 0x30 ) );
#else
			const pe::PEB* ptr;
			__asm__ __volatile__( "mov %%fs:0x30, %0" : "=r"( ptr ) );
			return ptr;
#endif
#elif defined(_M_ARM) || defined(__arm__)
			return *reinterpret_cast< const pe::PEB** >( _MoveFromCoprocessor( 15, 0, 13, 0, 2 ) + 0x30 );
#elif defined(_M_ARM64) || defined(__aarch64__)
			return *reinterpret_cast< const pe::PEB** >( __getReg( 18 ) + 0x60 );
#elif defined(_M_IA64) || defined(__ia64__)
			return *reinterpret_cast< const pe::PEB** >( static_cast< char* >( _rdteb( ) ) + 0x60 );
#else
#error Unsupported platform. Open an issue and Ill probably add support.
#endif
		}

		LAZY_IMPORTER_FORCEINLINE const pe::PEB_LDR_DATA* ldr( ) {
			return reinterpret_cast< const pe::PEB_LDR_DATA* >( peb( )->m_loader_data );
		}

		LAZY_IMPORTER_FORCEINLINE const pe::IMAGE_NT_HEADERS* nt_headers(
			const char* base ) noexcept {
			return reinterpret_cast< const pe::IMAGE_NT_HEADERS* >(
				base + reinterpret_cast< const pe::IMAGE_DOS_HEADER* >( base )->m_e_lfanew );
		}

		LAZY_IMPORTER_FORCEINLINE const pe::IMAGE_EXPORT_DIRECTORY* image_export_dir(
			const char* base ) noexcept {
			return reinterpret_cast< const pe::IMAGE_EXPORT_DIRECTORY* >(
				base + nt_headers( base )->m_optional_header.m_data_directory->m_virtual_address );
		}

		LAZY_IMPORTER_FORCEINLINE const pe::LDR_DATA_TABLE_ENTRY* ldr_data_entry( ) noexcept {
			return reinterpret_cast< const pe::LDR_DATA_TABLE_ENTRY* >(
				ldr( )->m_in_load_order_module_list.m_flink );
		}

		struct exports_directory {
			unsigned long                      _ied_size;
			const char* _base;
			const pe::IMAGE_EXPORT_DIRECTORY* _ied;

		public:
			using size_type = unsigned long;

			LAZY_IMPORTER_FORCEINLINE
				exports_directory( const char* base ) noexcept : _base( base ) {
				const auto ied_data_dir = nt_headers( base )->m_optional_header.m_data_directory[ 0 ];
				_ied = reinterpret_cast< const pe::IMAGE_EXPORT_DIRECTORY* >(
					base + ied_data_dir.m_virtual_address );
				_ied_size = ied_data_dir.m_size;
			}

			LAZY_IMPORTER_FORCEINLINE explicit operator bool( ) const noexcept {
				return reinterpret_cast< const char* >( _ied ) != _base;
			}

			LAZY_IMPORTER_FORCEINLINE size_type size( ) const noexcept {
				return _ied->m_number_of_names;
			}

			LAZY_IMPORTER_FORCEINLINE const char* base( ) const noexcept { return _base; }
			LAZY_IMPORTER_FORCEINLINE const pe::IMAGE_EXPORT_DIRECTORY* ied( ) const noexcept {
				return _ied;
			}

			LAZY_IMPORTER_FORCEINLINE const char* name( size_type index ) const noexcept {
				return _base + reinterpret_cast< const unsigned long* >( _base + _ied->m_address_of_names )[ index ];
			}

			LAZY_IMPORTER_FORCEINLINE const char* address( size_type index ) const noexcept {
				const auto* const rva_table =
					reinterpret_cast< const unsigned long* >( _base + _ied->m_address_of_functions );

				const auto* const ord_table = reinterpret_cast< const unsigned short* >(
					_base + _ied->m_address_of_name_ordinals );

				return _base + rva_table[ ord_table[ index ] ];
			}

			LAZY_IMPORTER_FORCEINLINE bool is_forwarded(
				const char* export_address ) const noexcept {
				const auto ui_ied = reinterpret_cast< const char* >( _ied );
				return ( export_address > ui_ied && export_address < ui_ied + _ied_size );
			}
		};

		struct safe_module_enumerator {
			using value_type = const pe::LDR_DATA_TABLE_ENTRY;
			value_type* value;
			value_type* head;

			LAZY_IMPORTER_FORCEINLINE safe_module_enumerator( ) noexcept
				: safe_module_enumerator( ldr_data_entry( ) ) {}

			LAZY_IMPORTER_FORCEINLINE
				safe_module_enumerator( const pe::LDR_DATA_TABLE_ENTRY* ldr ) noexcept
				: value( ldr->load_order_next( ) ), head( value ) {}

			LAZY_IMPORTER_FORCEINLINE void reset( ) noexcept {
				value = head->load_order_next( );
			}

			LAZY_IMPORTER_FORCEINLINE bool next( ) noexcept {
				value = value->load_order_next( );

				return value != head && value->m_dll_base;
			}
		};

		struct unsafe_module_enumerator {
			using value_type = const pe::LDR_DATA_TABLE_ENTRY*;
			value_type value;

			LAZY_IMPORTER_FORCEINLINE unsafe_module_enumerator( ) noexcept
				: value( ldr_data_entry( ) ) {}

			LAZY_IMPORTER_FORCEINLINE void reset( ) noexcept { value = ldr_data_entry( ); }

			LAZY_IMPORTER_FORCEINLINE bool next( ) noexcept {
				value = value->load_order_next( );
				return true;
			}
		};


		template<class Derived, class DefaultType = void*>
		class lazy_base {
		protected:


			LAZY_IMPORTER_FORCEINLINE static void*& _cache( ) noexcept {
				static void* value = nullptr;
				return value;
			}

		public:
			template<class T = DefaultType>
			LAZY_IMPORTER_FORCEINLINE static T safe( ) noexcept {
				return Derived::template get<T, safe_module_enumerator>( );
			}

			template<class T = DefaultType, class Enum = unsafe_module_enumerator>
			LAZY_IMPORTER_FORCEINLINE static T cached( ) noexcept {
				auto& cached = _cache( );
				if ( !cached )
					cached = Derived::template get<void*, Enum>( );

				return ( T ) ( cached );
			}

			template<class T = DefaultType>
			LAZY_IMPORTER_FORCEINLINE static T safe_cached( ) noexcept {
				return cached<T, safe_module_enumerator>( );
			}
		};

		template<offset_hash_pair OHP>
		struct lazy_module : lazy_base<lazy_module<OHP>> {
			template<class T = void*, class Enum = unsafe_module_enumerator>
			LAZY_IMPORTER_FORCEINLINE static T get( ) noexcept {
				Enum e{};
				do {
					if ( hash( e.value->m_base_dll_name, get_offset( OHP ) ) == get_hash( OHP ) )
						return ( T ) ( e.value->m_dll_base );
				} while ( e.next( ) );
				return {};
			}

			template<class T = void*, class Ldr>
			LAZY_IMPORTER_FORCEINLINE static T in( Ldr ldr ) noexcept {
				safe_module_enumerator e( reinterpret_cast< const pe::LDR_DATA_TABLE_ENTRY* >( ldr ) );
				do {
					if ( hash( e.value->m_base_dll_name, get_offset( OHP ) ) == get_hash( OHP ) )
						return ( T ) ( e.value->m_dll_base );
				} while ( e.next( ) );
				return {};
			}

			template<class T = void*, class Ldr>
			LAZY_IMPORTER_FORCEINLINE static T in_cached( Ldr ldr ) noexcept {
				auto& cached = lazy_base<lazy_module<OHP>>::_cache( );
				if ( !cached )
					cached = in( ldr );

				return ( T ) ( cached );
			}
		};

		template<offset_hash_pair OHP, class T>
		struct lazy_function : lazy_base<lazy_function<OHP, T>, T> {
			using base_type = lazy_base<lazy_function<OHP, T>, T>;

			template<class... Args>
			LAZY_IMPORTER_FORCEINLINE decltype( auto ) operator()( Args&&... args ) const {
#ifndef LAZY_IMPORTER_CACHE_OPERATOR_PARENS
				return get( )( LAZY_IMPORTER_CPP_FORWARD( Args, args )... );
#else
				return this->cached( )( LAZY_IMPORTER_CPP_FORWARD( Args, args )... );
#endif
			}

			template<class F = T, class Enum = unsafe_module_enumerator>
			LAZY_IMPORTER_FORCEINLINE static F get( ) noexcept {


#ifdef LAZY_IMPORTER_RESOLVE_FORWARDED_EXPORTS
				return forwarded<F, Enum>( );
#else

				Enum e{};
				do {
#ifdef LAZY_IMPORTER_HARDENED_MODULE_CHECKS
					if ( !e.value->m_dll_base || !e.value->m_full_dll_name.m_length )
						continue;
#endif

					const exports_directory exports( e.value->m_dll_base );

					if ( exports ) {
						auto export_index = exports.size( );
						while ( export_index-- )
							if ( hash( exports.name( export_index ), get_offset( OHP ) ) == get_hash( OHP ) )
								return ( F ) ( exports.address( export_index ) );
					}
				} while ( e.next( ) );
				return {};
#endif
			}

			template<class F = T, class Enum = unsafe_module_enumerator>
			LAZY_IMPORTER_FORCEINLINE static F forwarded( ) noexcept {
				pe::UNICODE_STRING name;
				forwarded_hashes              hashes{ 0, get_hash( OHP ) };

				Enum e{};
				do {
					name = e.value->m_base_dll_name;
					name.m_length -= 8;

					if ( !hashes.module_hash || hash( name, get_offset( OHP ) ) == hashes.module_hash ) {
						const exports_directory exports( e.value->m_dll_base );

						if ( exports ) {
							auto export_index = exports.size( );
							while ( export_index-- )
								if ( hash( exports.name( export_index ), get_offset( OHP ) ) == hashes.function_hash ) {
									const auto addr = exports.address( export_index );

									if ( exports.is_forwarded( addr ) ) {
										hashes = hash_forwarded(
											reinterpret_cast< const char* >( addr ),
											get_offset( OHP ) );

										e.reset( );
										break;
									}
									return ( F ) ( addr );
								}
						}
					}
				} while ( e.next( ) );
				return {};
			}

			template<class F = T>
			LAZY_IMPORTER_FORCEINLINE static F forwarded_safe( ) noexcept {
				return forwarded<F, safe_module_enumerator>( );
			}

			template<class F = T, class Enum = unsafe_module_enumerator>
			LAZY_IMPORTER_FORCEINLINE static F forwarded_cached( ) noexcept {
				auto& value = base_type::_cache( );
				if ( !value )
					value = forwarded<void*, Enum>( );
				return ( F ) ( value );
			}

			template<class F = T>
			LAZY_IMPORTER_FORCEINLINE static F forwarded_safe_cached( ) noexcept {
				return forwarded_cached<F, safe_module_enumerator>( );
			}

			template<class F = T, bool IsSafe = false, class Module>
			LAZY_IMPORTER_FORCEINLINE static F in( Module m ) noexcept {
				if ( IsSafe && !m )
					return {};

				const exports_directory exports( ( const char* ) ( m ) );
				if ( IsSafe && !exports )
					return {};

				for ( unsigned long i{};; ++i ) {
					if ( IsSafe && i == exports.size( ) )
						break;

					if ( hash( exports.name( i ), get_offset( OHP ) ) == get_hash( OHP ) )
						return ( F ) ( exports.address( i ) );
				}
				return {};
			}

			template<class F = T, class Module>
			LAZY_IMPORTER_FORCEINLINE static F in_safe( Module m ) noexcept {
				return in<F, true>( m );
			}

			template<class F = T, bool IsSafe = false, class Module>
			LAZY_IMPORTER_FORCEINLINE static F in_cached( Module m ) noexcept {
				auto& value = base_type::_cache( );
				if ( !value )
					value = in<void*, IsSafe>( m );
				return ( F ) ( value );
			}

			template<class F = T, class Module>
			LAZY_IMPORTER_FORCEINLINE static F in_safe_cached( Module m ) noexcept {
				return in_cached<F, true>( m );
			}

			template<class F = T>
			LAZY_IMPORTER_FORCEINLINE static F nt( ) noexcept {
				return in<F>( ldr_data_entry( )->load_order_next( )->m_dll_base );
			}

			template<class F = T>
			LAZY_IMPORTER_FORCEINLINE static F nt_safe( ) noexcept {
				return in_safe<F>( ldr_data_entry( )->load_order_next( )->m_dll_base );
			}

			template<class F = T>
			LAZY_IMPORTER_FORCEINLINE static F nt_cached( ) noexcept {
				return in_cached<F>( ldr_data_entry( )->load_order_next( )->m_dll_base );
			}

			template<class F = T>
			LAZY_IMPORTER_FORCEINLINE static F nt_safe_cached( ) noexcept {
				return in_safe_cached<F>( ldr_data_entry( )->load_order_next( )->m_dll_base );
			}
		};

	}
}

#endif 
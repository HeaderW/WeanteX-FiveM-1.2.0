#pragma once

namespace pe {
	struct UNICODE_STRING {
		uint16_t	m_length{};
		uint16_t	m_maximum_length{};
		wchar_t*	m_buffer{};
	};

	struct OBJECT_ATTRIBUTES {
		OBJECT_ATTRIBUTES( ) = default;
		OBJECT_ATTRIBUTES( UNICODE_STRING* name, ULONG attributes, HANDLE root, PVOID security, PVOID security_quality) {
			m_length						= sizeof( OBJECT_ATTRIBUTES );
			m_root_directory				= root;
			m_object_name					= name;
			m_attributes					= attributes;
			m_security_descriptor			= security;
			m_security_quality_of_service	= security_quality;
		}

		ULONG           m_length{};
		HANDLE          m_root_directory{};
		UNICODE_STRING* m_object_name{};
		ULONG           m_attributes{};
		PVOID           m_security_descriptor{};
		PVOID           m_security_quality_of_service{};
	};

	struct CLIENT_ID {
		PVOID m_unique_process{};
		PVOID m_unique_thread{};
	};

	struct SYSTEM_HANDLE_TABLE_ENTRY_INFO {
		ULONG			m_process_id{};
		BYTE			m_object_type_number{};
		BYTE			m_flags{};
		USHORT			m_handle{};
		PVOID			m_object{};
		ACCESS_MASK		m_granted_access{};
	};

	struct SYSTEM_HANDLE_INFORMATION {
		ULONG							m_handle_count{};
		SYSTEM_HANDLE_TABLE_ENTRY_INFO	m_handles[ 1 ]{};
	};

	struct LIST_ENTRY {
		struct LIST_ENTRY* m_flink{};
		struct LIST_ENTRY* m_blink{};
	};

	struct LDR_DATA_TABLE_ENTRY {
		LIST_ENTRY		m_in_load_order_links{};
		LIST_ENTRY		m_in_memory_order_links{};
		LIST_ENTRY		m_in_initialization_order_links{};
		const char*		m_dll_base{};
		const char*		m_entry_point{};
		unsigned long	m_size_of_image{};
		const char*		m_dummy{};
		UNICODE_STRING  m_full_dll_name{};
		UNICODE_STRING  m_base_dll_name{};
		unsigned long	m_check_sum{};
		void*			m_reserved6{};
		unsigned long	m_time_date_stamp{};

		__forceinline const LDR_DATA_TABLE_ENTRY* load_order_next( ) const noexcept { return reinterpret_cast< const LDR_DATA_TABLE_ENTRY* >( m_in_load_order_links.m_flink ); }
	};

	struct PEB_LDR_DATA {
		unsigned long	m_length{};
		unsigned char	m_initialized{};
		void*			m_ss_handle{};
		LIST_ENTRY		m_in_load_order_module_list{};
		LIST_ENTRY		m_in_memory_order_module_list{};
		LIST_ENTRY		m_in_initialization_order_module_list{};
	};

	struct PEB {
		UCHAR			m_inherited_address_space{};
		UCHAR			m_read_image_file_exec_options{};
		UCHAR			m_being_debugged{};
		UCHAR			m_bit_field{};
		PVOID			m_mutant{};
		PVOID			m_image_base_address{};
		PEB_LDR_DATA*	m_loader_data{};
	};

	struct IMAGE_EXPORT_DIRECTORY {
		unsigned long  m_characteristics{};
		unsigned long  m_time_date_stamp{};
		unsigned short m_major_version{};
		unsigned short m_minor_version{};
		unsigned long  m_name{};
		unsigned long  m_base{};
		unsigned long  m_number_of_functions{};
		unsigned long  m_number_of_names{};
		unsigned long  m_address_of_functions{};
		unsigned long  m_address_of_names{};
		unsigned long  m_address_of_name_ordinals{};
	};

	struct IMAGE_DOS_HEADER {
		unsigned short m_e_magic{};
		unsigned short m_e_cblp{};
		unsigned short m_e_cp{};
		unsigned short m_e_crlc{};
		unsigned short m_e_cparhdr{};
		unsigned short m_e_minalloc{};
		unsigned short m_e_maxalloc{};
		unsigned short m_e_ss{};
		unsigned short m_e_sp{};
		unsigned short m_e_csum{};
		unsigned short m_e_ip{};
		unsigned short m_e_cs{};
		unsigned short m_e_lfarlc{};
		unsigned short m_e_ovno{};
		unsigned short m_e_res[ 4 ]{};
		unsigned short m_e_oemid{};
		unsigned short m_e_oeminfo{};
		unsigned short m_e_res2[ 10 ]{};
		long           m_e_lfanew{};
	};

	struct IMAGE_FILE_HEADER {
		unsigned short m_machine{};
		unsigned short m_number_of_sections{};
		unsigned long  m_time_date_stamp{};
		unsigned long  m_pointer_to_symbol_table{};
		unsigned long  m_number_of_symbols{};
		unsigned short m_size_of_optional_header{};
		unsigned short m_characteristics{};
	};

	struct IMAGE_DATA_DIRECTORY {
		unsigned long m_virtual_address{};
		unsigned long m_size{};
	};

	struct IMAGE_OPTIONAL_HEADER64 {
		unsigned short			m_magic{};
		unsigned char			m_major_linker_version{};
		unsigned char			m_minor_linker_version{};
		unsigned long			m_size_of_code{};
		unsigned long			m_size_of_initialized_data{};
		unsigned long			m_size_of_uninitialized_data{};
		unsigned long			m_address_of_entry_point{};
		unsigned long			m_base_of_code{};
		unsigned long long		m_image_base{};
		unsigned long			m_section_alignment{};
		unsigned long			m_file_alignment{};
		unsigned short			m_major_operating_system_version{};
		unsigned short			m_minor_operation_system_version{};
		unsigned short			m_major_image_version{};
		unsigned short			m_minor_image_version{};
		unsigned short			m_major_subsystem_version{};
		unsigned short			m_minor_subsystem_version{};
		unsigned long			m_win32_version_value{};
		unsigned long			m_size_of_image{};
		unsigned long			m_size_of_headers{};
		unsigned long			m_check_sum{};
		unsigned short			m_subsystem{};
		unsigned short			m_dll_characteristics{};
		unsigned long long		m_size_of_stack_reserve{};
		unsigned long long		m_size_of_stack_commit{};
		unsigned long long		m_size_of_heap_reserve{};
		unsigned long long		m_size_of_heap_commit{};
		unsigned long			m_loader_flags{};
		unsigned long			m_number_of_rva_and_sizes{};
		IMAGE_DATA_DIRECTORY	m_data_directory[ 16 ]{};
	};

	struct IMAGE_OPTIONAL_HEADER32 {
		unsigned short       m_magic{};
		unsigned char        m_major_linker_version{};
		unsigned char        m_minor_linker_version{};
		unsigned long        m_size_of_code{};
		unsigned long        m_size_of_initialized_data{};
		unsigned long        m_size_of_uninitialized_data{};
		unsigned long        m_address_of_entry_point{};
		unsigned long        m_base_of_code{};
		unsigned long        m_base_of_data{};
		unsigned long        m_image_base{};
		unsigned long        m_section_alignment{};
		unsigned long        m_file_alignment{};
		unsigned short       m_major_operating_system_version{};
		unsigned short       m_minor_operation_system_version{};
		unsigned short       m_major_image_version{};
		unsigned short       m_minor_image_version{};
		unsigned short       m_major_subsystem_version{};
		unsigned short       m_minor_subsystem_version{};
		unsigned long        m_win32_version_value{};
		unsigned long        m_size_of_image{};
		unsigned long        m_size_of_headers{};
		unsigned long        m_check_sum{};
		unsigned short       m_subsystem{};
		unsigned short       m_dll_characteristics{};
		unsigned long        m_size_of_stack_reserve{};
		unsigned long        m_size_of_stack_commit{};
		unsigned long        m_size_of_heap_reserve{};
		unsigned long        m_size_of_heap_commit{};
		unsigned long        m_loader_flags{};
		unsigned long        m_number_of_rva_and_sizes{};
		IMAGE_DATA_DIRECTORY m_data_directory[ 16 ]{};
	};

	struct IMAGE_NT_HEADERS {
#if defined(_M_X64)
		using IMAGE_OPT_HEADER_ARCH = IMAGE_OPTIONAL_HEADER64;
#elif defined(_M_IX86)
		using IMAGE_OPT_HEADER_ARCH = IMAGE_OPTIONAL_HEADER32;
#endif
		unsigned long			m_signature{};
		IMAGE_FILE_HEADER		m_file_header{};
		IMAGE_OPT_HEADER_ARCH	m_optional_header{};
	};
}
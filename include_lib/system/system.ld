
    _lib_version_begin = .;
    PROVIDE(lib_version_begin = .);
        *(.lib_version)
    _lib_version_end = .;
    PROVIDE(lib_version_end = .);

    _sys_version_begin = .;
    PROVIDE(sys_version_begin = .);
        *(.sys.version)
    _sys_version_end = .;
    PROVIDE(sys_version_end = .);

        . = ALIGN(4);

	_initcall_begin = .;
	PROVIDE(initcall_begin = .);
		*(.initcall)
	_initcall_end = .;
	PROVIDE(initcall_end = .);

	_early_initcall_begin = .;
	PROVIDE(early_initcall_begin = .);
		*(.early.initcall)
	_early_initcall_end = .;
	PROVIDE(early_initcall_end = .);

	_late_initcall_begin = .;
	PROVIDE(late_initcall_begin = .);
	   *(.late.initcall)
	_late_initcall_end = .;
	PROVIDE(late_initcall_end = .);

	_platform_initcall_begin = .;
	PROVIDE(platform_initcall_begin = .);
	   *(.platform.initcall)
	_platform_initcall_end = .;
	PROVIDE(platform_initcall_end = .);

	_module_initcall_begin = .;
	PROVIDE(module_initcall_begin = .);
	   *(.module.initcall)
	_module_initcall_end = .;
	PROVIDE(module_initcall_end = .);

	_sys_event_handler_begin = .;
	PROVIDE(sys_event_handler_begin = .);
	   *(.sys_event.4.handler)
	   *(.sys_event.3.handler)
	   *(.sys_event.2.handler)
	   *(.sys_event.1.handler)
	   *(.sys_event.0.handler)
	_sys_event_handler_end = .;
	PROVIDE(sys_event_handler_end = .);

	_syscfg_arg_begin = .;
	PROVIDE(syscfg_arg_begin = .);
	   *(.syscfg.arg)
	_syscfg_arg_end = .;
	PROVIDE(syscfg_arg_end = .);

	_syscfg_handler_begin = .;
	PROVIDE(syscfg_handler_begin = .);
	   *(.syscfg.handler)
	_syscfg_handler_end = .;
	PROVIDE(syscfg_handler_end = .);

	_server_info_begin = .;
	PROVIDE(server_info_begin = .);
	   *(.server_info)
	_server_info_end = .;
	PROVIDE(server_info_end = .);

	_app_begin = .;
	PROVIDE(app_begin = .);
	   *(.app)
	_app_end = .;
	PROVIDE(app_end = .);

	_bus_device_begin = .;
	PROVIDE(bus_device_begin = .);
	   *(.bus_device)
	_bus_device_end = .;
	PROVIDE(bus_device_end = .);

    _sys_power_hal_ops_begin = .;
    PROVIDE(sys_power_hal_ops_begin = .);
        *(.sys_power_hal_ops)
    _sys_power_hal_ops_end = .;
    PROVIDE(sys_power_hal_ops_end = .);


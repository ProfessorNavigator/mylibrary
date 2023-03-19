project ('MyLibrary', 'cpp', default_options : ['cpp_std=c++17'])
subdir ('src')
subdir ('resources')
subdir ('include')
gtkmm = dependency ('gtkmm-4.0', method : 'pkg-config')
icu = dependency ('icu-i18n', method : 'pkg-config', version : '>=69')
zip = dependency ('libzip', method : 'pkg-config')
libarchive = dependency ('libarchive', method : 'pkg-config')
libgcrypt = dependency ('libgcrypt', method : 'pkg-config')
poppler = dependency ('poppler-cpp', method : 'pkg-config')
djvulibre = dependency ('ddjvuapi', method : 'pkg-config')
gmpxx = dependency ('gmpxx', method : 'pkg-config')
cpp = meson.get_compiler ('cpp')
thread_dep = dependency ('threads')

if poppler.version() < '22.01'
  	add_project_arguments('-D_OLDPOPPLER', language : 'cpp')
endif

if gtkmm.version() < '4.10'
	add_project_arguments('-DML_GTK_OLD', language : 'cpp')
endif

subdir ('po')
if gtkmm.version() < '4.10'
executable ('mylibrary', sources : [srcgui, srcresources], include_directories : [inc], dependencies : [gtkmm, icu, zip, libarchive, libgcrypt, poppler, djvulibre, gmpxx, thread_dep], install : true, win_subsystem : 'windows')
endif
if not gtkmm.version() < '4.10'
executable ('mylibrary', sources : [srcgui_new, srcresources], include_directories : [inc], dependencies : [gtkmm, icu, zip, libarchive, libgcrypt, poppler, djvulibre, gmpxx, thread_dep], install : true, win_subsystem : 'windows')
endif

install_subdir ('MyLibrary', install_dir : 'share')
if host_machine.system() == 'linux'
	install_data ('mylibrary.png', install_dir : 'share/icons/hicolor/256x256/apps')
	install_data ('ru.mail.bobilev_yury.MyLibrary.desktop', install_dir : 'share/applications')
endif
if host_machine.system() == 'windows'
	install_data ('mylibrary.ico', install_dir : 'share/icons/hicolor/256x256/apps')
endif	

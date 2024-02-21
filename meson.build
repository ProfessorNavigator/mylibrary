project ('MyLibrary', 'cpp', default_options : ['cpp_std=c++17'])
subdir ('src')
subdir ('include')
subdir ('po')
gtkmm = dependency ('gtkmm-4.0', method : 'pkg-config')
icu = dependency ('icu-i18n', method : 'pkg-config', version : '>=69')
libarchive = dependency ('libarchive', method : 'pkg-config')
libgcrypt = dependency ('libgcrypt', method : 'pkg-config')
gpg_error = dependency ('gpg-error', method : 'pkg-config')
poppler = dependency ('poppler-cpp', method : 'pkg-config')
djvulibre = dependency ('ddjvuapi', method : 'pkg-config')
cpp = meson.get_compiler ('cpp')
thread_dep = dependency ('threads')

poppler_version = poppler.version()
poppler_new = poppler_version.version_compare ('>=22.05')

if not poppler_new
  	add_project_arguments('-D_OLDPOPPLER', language : 'cpp')
endif

gtkmm_version = gtkmm.version()

gtkmm_new = gtkmm_version.version_compare ('>=4.10')

if not gtkmm_new
	add_project_arguments('-DML_GTK_OLD', language : 'cpp')
endif

executable ('mylibrary', sources : [srcgui], include_directories : [inc], dependencies : [gtkmm, icu, libarchive, libgcrypt, gpg_error, poppler, djvulibre, thread_dep], install : true, win_subsystem : 'windows')

install_subdir ('MyLibrary', install_dir : 'share')
if host_machine.system() == 'linux'
	install_data ('mylibrary.png', install_dir : 'share/icons/hicolor/256x256/apps')
endif
if host_machine.system() == 'windows'
	install_data ('mylibrary.ico', install_dir : 'share/icons/hicolor/256x256/apps')
endif
install_data ('ru.mail.bobilev_yury.MyLibrary.desktop', install_dir : 'share/applications')


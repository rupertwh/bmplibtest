project('bmplibtest', 'c', default_options: ['c_std=c11', 'warning_level=3'], version: '0.5.3')

cc = meson.get_compiler('c')

add_global_arguments('-pedantic', language : 'c')

conf_data = configuration_data()

conf_data.set('progname', meson.project_name())
conf_data.set('prog_version', meson.project_version())

if get_option('sanitize')
  sanitize = [
    '-fsanitize=signed-integer-overflow',
    '-fsanitize=undefined',
    '-fsanitize=float-divide-by-zero',
  ]
  add_project_arguments(sanitize, language : 'c')
  add_project_link_arguments(sanitize, language: 'c')
endif

#jpegdep = dependency('libjpeg',required: false)
pngdep = dependency('libpng')
bmpdep = dependency('libbmp')
#exifdep = dependency('libexif',required: false)
#lcms2dep = dependency('lcms2',required: false)

mathdep = cc.find_library('m')


conf_data.set('HAVE_LIBBMP', bmpdep.found()) 
conf_data.set('HAVE_LIBPNG', pngdep.found()) 

configure_file(input : 'config.h.in',
               output : 'config.h',
               configuration : conf_data)

if get_option('buildtype') == 'debug'
  add_global_arguments('-DDEBUG', language : 'c')
elif get_option('buildtype') == 'release'
  add_global_arguments('-DNDEBUG', language : 'c')
endif


executable('bmplibtest',
           'vartest.c',
           'imgstack.c',
           'testparser.c',
           'allocate.c',
           'conf.c',
           install: true,
           dependencies: [bmpdep, pngdep, mathdep]
)

executable('bmpinspect',
           'bmpinspect.c',
           install: true,
)

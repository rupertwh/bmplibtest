project('bmplibtest', 'c', default_options: ['c_std=c11'], version: '0.2.0')

cc = meson.get_compiler('c')

add_global_arguments('-pedantic', language : 'c')

conf_data = configuration_data()

conf_data.set('progname', meson.project_name())
conf_data.set('prog_version', meson.project_version())

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
endif


executable('vartest',
           'vartest.c',
           'imgstack.c',
           'cmdparser.c',
           dependencies: [bmpdep, pngdep, mathdep])

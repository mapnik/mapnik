
import re
import os
import sys
from copy import copy
from subprocess import Popen, PIPE

Import('env')

config_env = env.Clone()

# TODO
# major/minor versions
# git rev-list --max-count=1 HEAD

config_variables = '''#!/bin/sh

## variables

CONFIG_PREFIX="$( cd "$( dirname $( dirname "$0" ))" && pwd )"

CONFIG_MAPNIK_VERSION='%(version)s'
CONFIG_GIT_REVISION='%(git_revision)s'
CONFIG_GIT_DESCRIBE='%(git_describe)s'
CONFIG_FONTS="%(fonts)s"
CONFIG_INPUT_PLUGINS="%(input_plugins)s"
CONFIG_MAPNIK_DEFINES='%(defines)s'
CONFIG_MAPNIK_LIBNAME='%(mapnik_libname)s'
CONFIG_MAPNIK_LIBPATH="%(mapnik_libpath)s"
CONFIG_DEP_LIBS='%(dep_libs)s'
CONFIG_MAPNIK_LDFLAGS='%(ldflags)s'
CONFIG_MAPNIK_INCLUDE="${CONFIG_PREFIX}/include -I${CONFIG_PREFIX}/include/mapnik/agg"
CONFIG_DEP_INCLUDES='%(dep_includes)s'
CONFIG_CXXFLAGS='%(cxxflags)s'

'''

def write_config(configuration,template,config_file):
    template = open(template,'r').read()
    open(config_file,'w').write(config_variables  % configuration + template)
    try:
        os.chmod(config_file,0755)
    except: pass

cxxflags = ' '.join(config_env['LIBMAPNIK_CXXFLAGS'])

defines = ' '.join(config_env['LIBMAPNIK_DEFINES'])

dep_includes = ''.join([' -I%s' % i for i in config_env['CPPPATH'] if not i.startswith('#')])

dep_includes += ' '

if config_env['HAS_CAIRO']:
    dep_includes += ''.join([' -I%s' % i for i in env['CAIRO_CPPPATHS'] if not i.startswith('#')])

ldflags = config_env['CUSTOM_LDFLAGS'] + ''.join([' -L%s' % i for i in config_env['LIBPATH'] if not i.startswith('#')])

dep_libs = ''.join([' -l%s' % i for i in env['LIBMAPNIK_LIBS']])

# remove local agg from public linking
dep_libs = dep_libs.replace('-lagg','')

git_revision = 'unknown'
git_describe = 'unknown'
# special GIT_REVISION/GIT_DESCRIBE files present only for official releases
# where the git directory metadata is stripped
# more info: https://github.com/mapnik/mapnik/wiki/MapnikReleaseSteps
revision_release_file = '../../GIT_REVISION'
if os.path.exists(revision_release_file):
    git_revision = open(revision_release_file,'r').read()
else:
    git_cmd = "git rev-list --max-count=1 HEAD"
    stdin, stderr = Popen(git_cmd, shell=True, stdout=PIPE, stderr=PIPE).communicate()
    if not stderr:
        git_revision = stdin.strip()

describe_release_file = '../../GIT_DESCRIBE'
if os.path.exists(describe_release_file):
    git_describe = open(describe_release_file,'r').read()
else:
    git_cmd = "git describe"
    stdin, stderr = Popen(git_cmd, shell=True, stdout=PIPE, stderr=PIPE).communicate()
    if not stderr:
        git_describe = stdin.strip()

# for fonts and input plugins we should try
# to store the relative path, if feasible
fontspath = config_env['MAPNIK_FONTS']
lib_root = os.path.join(config_env['INSTALL_PREFIX'], config_env['LIBDIR_SCHEMA'])
if lib_root in fontspath:
    fontspath = "${CONFIG_PREFIX}/" + os.path.relpath(fontspath,config_env['INSTALL_PREFIX'])
inputpluginspath = config_env['MAPNIK_INPUT_PLUGINS']
if lib_root in inputpluginspath:
    inputpluginspath = "${CONFIG_PREFIX}/" + os.path.relpath(inputpluginspath,config_env['INSTALL_PREFIX'])

lib_path = "${CONFIG_PREFIX}/" + config_env['LIBDIR_SCHEMA']

configuration = {
    "git_revision": git_revision,
    "git_describe": git_describe,
    "version": config_env['MAPNIK_VERSION_STRING'],
    "mapnik_libname": 'mapnik',
    "mapnik_libpath": lib_path,
    "ldflags": ldflags,
    "dep_libs": dep_libs,
    "dep_includes": dep_includes,
    "fonts": fontspath,
    "input_plugins": inputpluginspath,
    "defines":defines,
    "cxxflags":cxxflags
}

## if we are statically linking depedencies
## then they do not need to be reported in ldflags
#if env['RUNTIME_LINK'] == 'static':
#    configuration['ldflags'] = ''
#    configuration['dep_libs'] = ''

template = 'mapnik-config.template.sh'
config_file = 'mapnik-config'
source = config_file
write_config(configuration,template,config_file)
target_path = os.path.normpath(os.path.join(config_env['INSTALL_PREFIX'],'bin'))
full_target = os.path.join(target_path,config_file)

Depends(full_target, env.subst('../../src/%s' % env['MAPNIK_LIB_NAME']))

if 'install' in COMMAND_LINE_TARGETS:
    # we must add 'install' catch here because otherwise
    # custom command will be run when not installing
    env.Alias('install',full_target)

    env.Command(full_target, config_file,
       [
       Copy("$TARGET","$SOURCE"),
       Chmod("$TARGET", 0755),
       ])

config_env['create_uninstall_target'](env,os.path.join(target_path,config_file))

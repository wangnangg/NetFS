#!/usr/bin/python3
import os.path


def list2makestr(objs):
    obj_str = []
    for obj in objs:
        obj_str.append(obj)
        obj_str.append(' ')
    return ''.join(obj_str)


# recursively list all files that match specified extensions
def find_files(target_dir, exts):
    if type(exts) is str:
        exts = [exts]
    files = []
    for dirpath, dirnames, filenames in os.walk(target_dir):
        for f in filenames:
            for e in exts:
                if (f.endswith(e)):
                    files.append(os.path.join(dirpath, f))
                    break

    return files


target_set = set()


def dir_rule(target):
    if target in target_set:
        return ''
    else:
        target_set.add(target)
        return "%s:\n\tmkdir -p $@\n" % target


def compile_rule(compiler, flags, src_file, target):
    if target in target_set:
        return ''
    else:
        target_set.add(target)
        target_path = os.path.dirname(target)
        return "%s: %s | %s \n\t%s %s -c %s -o %s\n" % (
            target, src_file, target_path, compiler, flags, src_file,
            target) + dir_rule(target_path)


def link_rule(linker, flags, objs, target):
    if target in target_set:
        return ''
    else:
        target_set.add(target)
        target_path = os.path.dirname(target)
        objs_str = list2makestr(objs)
        return "%s: %s | %s\n\t%s %s %s -o %s\n" % (
            target, objs_str, target_path, linker, objs_str, flags,
            target) + dir_rule(target_path)


def ar_rule(ar, flags, objs, target):
    if target in target_set:
        return ''
    else:
        target_set.add(target)
        target_path = os.path.dirname(target)
        objs_str = list2makestr(objs)
        return "%s: %s | %s\n\t%s %s %s %s\n" % (
            target, objs_str, target_path, ar, flags, target,
            objs_str) + dir_rule(target_path)


def change_ext(filename, new_ext):
    return os.path.splitext(filename)[0] + new_ext


def transform_src_files(source_files, obj_dir):
    objs = []
    for src in source_files:
        obj = os.path.join(obj_dir, src)
        obj = change_ext(obj, '.o')
        objs.append(obj)
    return objs


cpp_compiler = '${cpp_compiler}'
linker = '${linker}'
makefile_body = []

src_files = find_files('client_src', '.cpp')
obj_files = transform_src_files(src_files, "${build_dir}")
for i in range(0, len(src_files)):
    makefile_body.append(
        compile_rule(cpp_compiler, "${client_compile_flags}", src_files[i], obj_files[i]))

makefile_body.append(
    link_rule(linker, "${client_link_flags}",
              obj_files,
              '${build_dir}/client'))

deps = list2makestr(
    map(lambda obj: change_ext(obj, '.d'),
        obj_files))

makefile_body.append('deps:=' + deps + '\n')


with open('makefile.in', 'r') as fin:
    makefilein_content = fin.readlines()
    with open('makefile', 'w') as fmake:
        for line in makefilein_content:
            if line.startswith('#body'):
                fmake.write(''.join(makefile_body))
            else:
                fmake.write(line)
print('build with: make config=debug|release')

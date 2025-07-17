
pcut_dep = dependency('pcut', required: false)

src = files(['main.c','string_datastructure.c'])

if pcut_dep.found()
  test_sources = ['wildcardsTest.c', 'main.c']
  test_exe = executable('wildcards_test_runner',
    test_sources,
    dependencies: [pcut_dep],
    include_directories: ['.'])
  
  test('wildcards_test', test_exe)
endif
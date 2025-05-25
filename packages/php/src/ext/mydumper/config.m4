PHP_ARG_ENABLE(mydumper, whether to enable mydumper, [  --enable-mydumper   Enable mydumper ])

if test "$PHP_MYDUMPER" = "yes"; then
  PHP_NEW_EXTENSION(mydumper, mydumper.c, $ext_shared)
fi

#compdef connect

local -a _list
_list=(`connect --list-configs`)

_describe -t common-commands 'common commands' _list

arguments=(
    {-t,--protocol}'[Specify connection protocol]'
    {-u,--user}'[Specify username]'
    {-p,--pass}'[Specify password]'
    {-h,--host}'[Specify server host]'
    {-P,--port}'[Specify server port]'
    {-v,--volumn}'[Specify volumn]'
    '--help[Display help information]'
    '--version[Display version information]'
)

_arguments -s $arguments

#!/bin/sh

export QUEUE_NUM=42

alias iptables="LANG=C iptables"
alias ip6tables="LANG=C ip6tables"

stop() {
  local ipt
  for ipt in iptables ip6tables
  do
    $ipt -t mangle -L --line-numbers | sed "s/'//g" | awk '
      function c() {
	while (--n in NUMS) {
	  system("'$ipt' -t mangle -D '\''" cn "'\'' '\''" NUMS[n] "'\''")
	}
	cn = $2
      }
      $1=="Chain" {c()} END {c()}
      $2=="ptd_chain" && $1!="Chain" { NUMS[n++]=$1 }'
    $ipt -t mangle -F ptd_chain
    $ipt -t mangle -X ptd_chain
  done
}

start() {
  for ipt in iptables ip6tables; do
    $ipt -t mangle -N ptd_chain
    $ipt -t mangle -A OUTPUT -j ptd_chain
    $ipt -t mangle -A ptd_chain -p tcp --syn \
	-j NFQUEUE --queue-num $QUEUE_NUM --queue-bypass
  done
}

case "$1" in
  start) start;;
  stop) stop;;
  restart) stop; start;;

  *)
    echo "Usage: $0 {start|stop|restart}"
    exit 1;;
esac

exit 0

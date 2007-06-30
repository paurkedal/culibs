#! /bin/sh

usage()
{
    echo "$0 ?(--ask)"
    exit 1
}

ask=false
srcdir="`dirname $0`"

while [ $# -gt 0 ]; do
    case $1 in
	--ask)
	    ask=true
	    ;;
	*)
	    usage
	    ;;
    esac
    shift
done

if $ask; then
    cat <<EOF

  I will need the file UnicodeData.txt from http://www.unicode.org.  The
  URL is http://www.unicode.org/Public/UNIDATA/UnicodeData.txt.

EOF
    echo -n "Fetch it with wget? (y to confirm) "; read answer
    if [ "$answer" != "y" -a "$answer" != "yes" ]; then
	exit 1
    fi
fi

cd $srcdir
wget http://www.unicode.org/Public/UNIDATA/UnicodeData.txt

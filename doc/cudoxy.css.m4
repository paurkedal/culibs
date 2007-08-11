include(srcdir/doc/styledefs.m4)dnl
@import "cucommon.css";
body {
	background-color: white;
	color: black;
	margin-top: 0;
	margin-bottom: 0;
	margin-left: 8mm;
	margin-right: 8mm;
}
a.anchor { color: inherit; text-decoration: none; }
td.indexkey a.el { font-weight: bold; }
h3[align=center] { text-align: left; color: black; margin-bottom: 8mm; }
h3[align=center]:before { content: "Version "; color: black; }

body > ul > li > a.el { font-weight: bold; }

div.qindex {
	background-color: white;
	border: none;
}
div.tabs {
	margin: 0;  margin-left: -8mm; margin-right: -8mm;
	padding: 0; padding-left: 8mm; padding-right: 8mm;
	background-color: col_dark0;
	color: white;
}
div.tabs ul {
	list-style-type: none;
	padding: 0;
	margin: 0;
}
div.tabs li {
	display: inline;
	padding: 0;
}
div.tabs li + li { margin-left: 1em; }
dnl div.tabs li:before { content: "   "; }
dnl div.tabs li:after { content: "   "; }
div.tabs li.current { font-weight: bold; }
div.tabs li.current:before { content: "‹"; }
div.tabs li.current:after { content: "›"; }
div.tabs a:link, div.tabs a:visited { color: #eeeeee; }

hr {
	display: none;
}


dnl Doxygen-Specific Tags
dnl ---------------------

td.memItemLeft, td.memItemRight {
	background-color: white;
	border: none;
}

table.mdTable {
	width: 100%;
	margin-top: 3mm;
	margin-bottom: 0;
	margin-left: 0;
	border: none;
	background-color: white;
}
td.mdRow, td.md, td.mdname, td.mdname1 {
	border: none;
	background-color: inherit;
	padding: 0;
}
td.mdRow {
	padding-left: 3mm;
}
td.mdname em, td.mdname1 em {
	color: black;
	font-weight: normal;
}
table.mdTable + table {
	margin-left: 5mm;
}
td.memItemRight a.el:first-child {
	font-weight: bold;
}

div.fragment {
	margin-top: 2mm;
	margin-bottom: 2mm;
	margin-left: 5mm;
	margin-right: 5mm;
	padding: 0;
}
pre.fragment {
	background-color: #ffffee;
	border: thin solid #cccccc;
	padding: 1mm;
}

td.indexkey, td.indexvalue {
	background-color: #ffffff;
}
dnl div.memitem
dnl   div.memproto
dnl     table.memname
dnl       tr
dnl         td.memname
dnl         td.paramtype
dnl         td.paramname
dnl       tr
dnl         td.memkey
dnl         td.paramtype
dnl         td.paramname
dnl div.memdoc
dnl
dnl Not used: .memtemplate, .memnav
div.memproto {
	border-top: solid 1px col_dark0;
	border-left: solid 1mm col_dark0;
	margin-left: -3mm;  padding-left: 2mm;
	margin-right: -3mm; padding-right: 3mm;
	background-color: col_light0;
}
table.memname {
	font-family: monospace;
	width: 100%;
}
td.memname, td.memkey {
	padding-left: 0; padding-right: 0;
	margin-left: 0; padding-left: 0;
	white-space: nowrap;
}
td.paramtype {
	padding-left: 0; padding-right: 0;
	margin-left: 0; padding-left: 0;
	white-space: nowrap;
	font-family: monospace;
}
.memdoc {
	margin-top: 0.6em;
	margin-bottom: 1.6em;
}

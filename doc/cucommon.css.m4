include(srcdir/doc/styledefs.m4)dnl
body {
	background-color: white;
	color: black;
	margin: 8mm;
	margin-bottom: 0;
	margin-top: 0;
}
h2, h3, h4, h5, h6 {
	color: col_dark0;
	font-weight: bold;
}
h2 { border-bottom: solid thin col_dark0; }
a {
	color: col_dark1;
	text-decoration: none;
}
a:hover {
	text-decoration: underline;
}
ul { list-style: square; }

address, ul.crumbs {
	background-color: col_dark0;
	color: white;
	margin-left: -8mm;
	margin-right: -8mm;
	padding-left: 8mm;
	padding-right: 8mm;
	padding-top: 0.5mm;
	padding-bottom: 0.5mm;
}
address a, ul.crumbs a {
	color: white;
	text-decoration: underline;
}
address { margin-top: 6mm; }
ul.crumbs {
	margin-top: 0;
	list-style: none;
}
ul.crumbs li {
	display: inline;
}
ul.crumbs li:before {
	content: " | ";
}
ul.crumbs li:first-child:before {
	content: "";
}
dnl vim: filetype=css

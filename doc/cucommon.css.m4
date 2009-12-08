include(srcdir/doc/styledefs.m4)dnl
body {
	background-color: white;
	color: black;
	margin: 8mm;
	margin-bottom: 0;
	margin-top: 0;
}
h2, h3, h4, h5, h6 {
	color: COL0_DARK;
	font-weight: bold;
}
h2 { border-bottom: solid thin COL0_DARK; }
a {
	color: COL1_DARK;
	text-decoration: none;
}
a:hover {
	text-decoration: underline;
}
ul { list-style: square; }

address, ul.crumbs {
	background-color: COL0_DARK;
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
table.normal {
	margin: 0;
	margin-left: 8mm;
	margin-right: 8mm;
	padding: 0;
	border-collapse: collapse;
	border: none;
}
table.normal tr { margin: 0; padding: 0; }
table.normal td, table.normal th {
	margin: 0;
	padding: 0;
	padding-left: 1mm;
	padding-right: 1mm;
	border: thin solid COL0_MEDI;
	vertical-align: top;
}
table.normal th {
	background-color: COL0_LIGHT;
	text-align: left;
}
/* Doxygen 1.6.1 inserts paragraphs within <td> elements, which cause a lot of
 * unnecessary space. */
table.normal td p:first-child, table.normal th p:first-child { margin-top: 0; }
table.normal td p, table.normal th p { margin-bottom: 0; }

dnl vim: filetype=css

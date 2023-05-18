#!/usr/bin/perl
use strict;
use warnings;
use CGI;

my $cgi = CGI->new;

print $cgi->header('text/html');
print <<EOF;
<html>
<head>
    <title>Test Page</title>
    <style>
        body {
            font-family: 'Courier New', Courier, monospace;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
            /* Gradient background */
            background: linear-gradient(to right, #12c2e9, #c471ed, #f64f59);
        }
        h1 {
            font-size: 3em;
            color: #FFFFFF;  /* Changing text color to white for better contrast against the gradient background */
        }
    </style>
</head>
<body>
    <h1>Hello, World!</h1>
</body>
</html>
EOF

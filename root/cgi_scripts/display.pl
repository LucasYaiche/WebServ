#!/usr/bin/perl
use strict;
use warnings;
use CGI;

my $cgi = CGI->new;

my $port = $ENV{'SERVER_PORT'}; # Access the environment variable

print $cgi->header('text/html');
print <<EOF;
<html>
<head>
    <title>Image Display</title>
    <style>
        body {
            /* Gradient background similar to space */
            background: linear-gradient(to right, #0b3c5d, #000000);
            font-family: Arial, sans-serif;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
        }
        img {
            border-radius: 15px;
            box-shadow: 0px 0px 30px rgba(0, 0, 0, 0.5);
            /* Adjusted image size to 35% of viewport width */
            width: 35%;
            height: auto;
        }
    </style>
</head>
<body>
    <img src="http://localhost:$port/uploads/minion.jpeg" alt="Image">
</body>
</html>
EOF

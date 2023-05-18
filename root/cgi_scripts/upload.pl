#!/usr/bin/perl
use strict;
use warnings;
use CGI;

my $cgi = CGI->new;

# Check if this is a POST request
if ($cgi->request_method() eq 'POST') {
    # Get the names of all uploaded files
    my @filenames = $cgi->upload_info();

    # Respond with the names of the uploaded files
    print $cgi->header('text/html');
    print "<p>Uploaded files:</p>";
    print "<ul>";
    foreach my $filename (@filenames) {
        print "<li>$filename</li>";
    }
    print "</ul>";
} else {
    # This is a GET request, so display the upload form
    print $cgi->header('text/html');
    print <<EOF;
<form method="POST" enctype="multipart/form-data">
    <input type="file" name="file">
    <input type="submit" value="Upload">
</form>
EOF
}

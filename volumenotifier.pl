#!/usr/pkg/bin/perl

use strict;
use Mac::Glue;

my $notifications = [
	'volume'
];

$notifications = [ map {
	Mac::Glue::param_type(Mac::AppleEvents::typeChar(), $_)
} @$notifications ];

sub strp($) {
	my ($string) = @_;
	return Mac::Glue::param_type(Mac::Glue::typeUnicodeText(), $string);
}


my $glue = Mac::Glue->new('GrowlHelperApp');

$glue->register('as_application' => strp("Marantz"), 'all_notifications' => $notifications, 'default_notifications' => $notifications);

open MAR, '-|', '/usr/local/bin/marantz listen volume';

while (<MAR>) {
	/^volume (.*)/ or next;

	$glue->notify('with_name' => strp('volume'), 'title' => strp('Volume'), 'description' => strp("$1 dB"), 'application_name' => strp('Marantz'), 'image_from_location' => strp('file:///usr/local/share/sound-icon.png'), 'identifier' => strp('org.morth.pelle.marantz.volume'));
}

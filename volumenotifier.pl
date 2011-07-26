#!/usr/bin/env perl
# 
# Copyright (c) 2008 Pelle Johansson
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 

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

$glue->register('as_application' => strp("Movactl"), 'all_notifications' => $notifications, 'default_notifications' => $notifications);

open MAR, '-|', '/usr/local/bin/movactl listen volume';

while (<MAR>) {
	/^volume (.*)/ or next;

	$glue->notify('with_name' => strp('volume'), 'title' => strp('Volume'), 'description' => strp("$1 dB"), 'application_name' => strp('Movactl'), 'image_from_location' => strp('file:///usr/local/share/sound-icon.png'), 'identifier' => strp('org.morth.pelle.movactl.volume'));
}

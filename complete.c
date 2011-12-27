/*
 * Copyright (c) 2011 Pelle Johansson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "complete.h"

#include <string.h>

static void
prune_inexact(struct complete_candidate **cands, void (*elim_cb)(struct complete_candidate *cand)) {
	struct complete_candidate *cand, *pcand;

	pcand = NULL;
	cand = *cands;
	while (cand) {
		if (!cand->is_exact) {
			if (pcand)
				pcand->next = cand->next;
			else
				*cands = cand->next;
			elim_cb(cand);
			if (pcand)
				cand = pcand->next;
			else
				cand = *cands;
		} else {
			pcand = cand;
			cand = cand->next;
		}
	}
}

int
complete(struct complete_candidate **cands, int argc, const char **argv,
		void (*elim_cb)(struct complete_candidate *cand)) {
	struct complete_candidate *cand, *pcand;
	int argi = 0;
	int argo = 0;

	while (argi < argc) {
		const char *arg = argv[argi] + argo;
		const char *ea;
		int al;
		int have_exact = 0;

		ea = strpbrk (arg, "_- ");
		if (ea) {
			argo += ea - arg + 1;
			al = ea - arg;
		} else {
			argi++;
			argo = 0;
			al = strlen (arg);
		}

		pcand = NULL;
		cand = *cands;
		while (cand) {
			if (strncmp (arg, cand->name + cand->name_off, al) != 0) {
				if (pcand)
					pcand->next = cand->next;
				else
					*cands = cand->next;
				elim_cb(cand);
				if (pcand)
					cand = pcand->next;
				else
					cand = *cands;
			} else {
				ea = strpbrk (cand->name + cand->name_off, "_- ");
				if (ea) {
					cand->is_exact = ea - cand->name - cand->name_off == al;
					cand->name_off = ea - cand->name + 1;
				} else {
					cand->is_exact = (int)strlen (cand->name + cand->name_off) == al;
					cand->name_off += strlen (cand->name + cand->name_off);
				}
				if (cand->is_exact)
					have_exact = 1;
				pcand = cand;
				cand = cand->next;
			}
		}

		if (have_exact)
			prune_inexact(cands, elim_cb);

		if (!*cands)
			return argi;
 		if ( !(*cands)->next && !argo)
			return argi;
	}

	if (!*cands)
		return argi;

	if ((*cands)->next) {
		int have_complete = 0;
		/* See if any candidate is complete */
		for (cand = *cands; cand; cand = cand->next) {
			//fprintf (stderr, "Checking %s for completness, name_off = %d\n", cand->name, cand->name_off);
			if (cand->name_off == (int)strlen (cand->name)) {
				cand->is_exact = 1;
				have_complete = 1;
			} else {
				cand->is_exact = 0;
			}
		}
		if (have_complete) {
			prune_inexact(cands, elim_cb);
		}
	}
	return argi;
}

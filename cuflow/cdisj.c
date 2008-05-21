/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2007  Petter Urkedal <urkedal@nbi.dk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cuflow/cdisj.h>
#include <cuflow/workers.h>
#include <cu/hash.h>
#include <cu/thread.h>

#define CDISJ_STRIPE_COUNT 128
#define CDISJ_STRIPE_MASK (CDISJ_STRIPE_COUNT - 1)

typedef struct cdisj_stripe_s *cdisj_stripe_t;
struct cdisj_stripe_s
{
    AO_t waiting_count;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

static struct cdisj_stripe_s cdisj_stripe_arr[CDISJ_STRIPE_COUNT];

CU_SINLINE cdisj_stripe_t
cdisj_stripe(AO_t *cdisj)
{
    cu_hash_t i = cu_hash_mix((cu_hash_t)cdisj);
    return &cdisj_stripe_arr[i & CDISJ_STRIPE_MASK];
}

void
cuflowP_cdisj_broadcast(cuflow_cdisj_t *cdisj)
{
    cdisj_stripe_t stripe = cdisj_stripe(cdisj);
    if (AO_load(&stripe->waiting_count)) {
	cu_mutex_lock(&stripe->mutex);
	pthread_cond_broadcast(&stripe->cond);
	cu_mutex_unlock(&stripe->mutex);
    }
}

void
cuflowP_cdisj_wait_while(cuflow_cdisj_t *cdisj, cu_bool_t cond_val)
{
    cdisj_stripe_t stripe;
    cuflow_yield();
    stripe = cdisj_stripe(cdisj);
    cu_mutex_lock(&stripe->mutex);
    AO_fetch_and_add1(&stripe->waiting_count);
    while (!!AO_load(cdisj) != cond_val)
	pthread_cond_wait(&stripe->cond, &stripe->mutex);
    AO_fetch_and_sub1(&stripe->waiting_count);
    cu_mutex_unlock(&stripe->mutex);
}

void
cuflowP_cdisj_init(void)
{
    int i;
    for (i = 0; i < CDISJ_STRIPE_COUNT; ++i) {
	cdisj_stripe_t stripe = &cdisj_stripe_arr[i];
	stripe->waiting_count = 0;
	cu_mutex_init(&stripe->mutex);
	pthread_cond_init(&stripe->cond, NULL);
    }
}

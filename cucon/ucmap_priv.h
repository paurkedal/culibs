/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2010  Petter Urkedal <paurkedal@eideticdew.org>
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

#ifndef CUCON_UCMAP_PRIV_H
#define CUCON_UCMAP_PRIV_H

CU_BEGIN_DECLARATIONS

CU_SINLINE cucon_ucmap_t
cucon_ucmap_node_left(cucon_ucmap_t node)
{ return ((cucon_ucmap_t)((uintptr_t)node->left & ~(uintptr_t)1)); }

CU_SINLINE cucon_ucmap_t
cucon_ucmap_node_right(cucon_ucmap_t node)
{ return (node->right); }

CU_SINLINE cu_bool_t
cucon_ucmap_node_has_value(cucon_ucmap_t node)
{ return ((uintptr_t)node->left & 1); }

CU_SINLINE uintptr_t
cucon_ucmap_node_value(cucon_ucmap_t node)
{ return (node->value); }

CU_SINLINE void *
cucon_ucmap_node_value_ptr(cucon_ucmap_t node)
{ return ((void *)node->value); }

CU_SINLINE uintptr_t
cucon_ucmap_node_key(cucon_ucmap_t node)
{ return node->key; }

CU_END_DECLARATIONS

#endif

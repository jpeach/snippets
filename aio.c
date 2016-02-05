/*
 *  Copyright 2015 James Peach
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#undef _NDEBUG
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <libaio.h>

int main(int argc, const char ** argv)
{
    io_context_t io;

    (void)argc;
    (void)argv;

    assert(io_queue_init(512, &io) == 0);
    assert(io_queue_run(io) == 0);
    assert(io_queue_release(io) == 0);

    return 0;
}


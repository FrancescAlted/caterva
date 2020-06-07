//
// Created by Aleix Alcacer Sales on 07/06/2020.
//

#ifndef CATERVA_TEST_COMMON_H
#define CATERVA_TEST_COMMON_H

#include <caterva.h>
#include "minunit/minunit.h"

#define MU_ASSERT_CATERVA(rc) do{ MU_ASSERT("Caterva error\n", rc == CATERVA_SUCCEED);} while(0)

#endif //CATERVA_TEST_COMMON_H

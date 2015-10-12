#include <tetrapol/tetrapol.h>

#include <stdlib.h>
#include <string.h>

struct tetrapol_priv_t {
    tetrapol_cfg_t cfg;
};

tetrapol_t *tetrapol_create(const tetrapol_cfg_t *cfg)
{
    tetrapol_t *tetrapol = malloc(sizeof(tetrapol_t));
    if (!tetrapol) {
        return NULL;
    }

    memcpy(&tetrapol->cfg, cfg, sizeof(tetrapol_cfg_t));

    return tetrapol;
}

void tetrapol_destroy(tetrapol_t *tetrapol)
{
    free(tetrapol);
}

const tetrapol_cfg_t *tetrapol_get_cfg(tetrapol_t *tetrapol)
{
    return &tetrapol->cfg;
}

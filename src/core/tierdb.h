#ifndef GMCORE_TIERDB_H
#define GMCORE_TIERDB_H

/* General */
void    tierdb_init     (DB_Table*);
BOOLEAN tierdb_reinit (DB_Table*);
void tierdbFreeChildTiers();
int CheckTierDB     (TIER, int);
BOOLEAN tierdb_load_minifile (char*);
void tierdbGetValueAndRemoteness(POSITION pos, VALUE *v, REMOTENESS *r);
void tierdbSetValueAndRemoteness(POSITION pos, VALUE v, REMOTENESS r);

#endif /* GMCORE_TIERDB_H */

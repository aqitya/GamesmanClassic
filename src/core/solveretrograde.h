#ifndef GMCORE_SOLVERETROGRADE_H
#define GMCORE_SOLVERETROGRADE_H

VALUE DetermineRetrogradeValue(POSITION);
POSITION InitTierGamesman();
void dedupHashExpand();
BOOLEAN dedupHashAdd(POSITION pos);

#endif /* GMCORE_SOLVERETROGRADE_H */

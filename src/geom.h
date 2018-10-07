#ifndef GEOM_H_
#define GEOM_H_

bool init_geom();
void destroy_geom();

void faros();
void ground();
void xlogo(float sz, const float *col_ink, const float *col_paper, float alpha, float xcircle);

#endif	/* GEOM_H_ */


// render.cpp [pengine]

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)


#include "pengine.h"



PSSRender::PSSRender(PApp &parentApp) : PSubsystem(parentApp)
{
  app.outLog() << "Initialising render subsystem" << std::endl;
}

PSSRender::~PSSRender()
{
  app.outLog() << "Shutting down render subsystem" << std::endl;
}


void PSSRender::tick(float delta, const vec3f &eyepos, const mat44f &eyeori, const vec3f &eyevel)
{
  float unused = delta; unused = eyevel.x;
  
  cam_pos = eyepos;
  cam_orimat = eyeori;
}


void PSSRender::render(PParticleSystem *psys)
{
  vec3f pushx = makevec3f(cam_orimat.row[0]);
  vec3f pushy = makevec3f(cam_orimat.row[1]);
  vec3f vert;
  
  glBlendFunc(psys->blendparam1, psys->blendparam2);
  
  if (psys->tex) psys->tex->bind();
  else glDisable(GL_TEXTURE_2D);
  
#ifdef HAVE_GLES
  GLboolean glCol = glIsEnabled(GL_COLOR_ARRAY);
  GLboolean glTex = glIsEnabled(GL_TEXTURE_COORD_ARRAY);
  GLboolean glVtx = glIsEnabled(GL_VERTEX_ARRAY);
  if (!glCol)	glEnableClientState(GL_COLOR_ARRAY);
  if (!glTex)	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  if (!glVtx)	glEnableClientState(GL_VERTEX_ARRAY);
  GLshort tex[1024*2];
  vec3f vtx[1024];
  GLfloat col[1024*4];
  int idx=0;
  GLshort indices[1024*6/4];
  int indice=0;
  glVertexPointer(3, GL_FLOAT, 0, vtx);
  glTexCoordPointer(2, GL_SHORT, 0, tex);
  glColorPointer(4, GL_FLOAT, 0, col);
#else
  glBegin(GL_QUADS);
#endif
  for (unsigned int i=0; i<psys->part.size(); i++) {
    PParticle_s &part = psys->part[i];
    float sizenow = INTERP(psys->endsize, psys->startsize, part.life);
    vec3f pushxt = pushx * sizenow;
    vec3f pushyt = pushy * sizenow;
    vec3f pushx2 = pushxt * part.orix.x + pushyt * part.orix.y;
    vec3f pushy2 = pushxt * part.oriy.x + pushyt * part.oriy.y;
#ifdef HAVE_GLES
	col[idx*4+0]=INTERP(psys->colorend[0], psys->colorstart[0], part.life);
	col[idx*4+1]=INTERP(psys->colorend[1], psys->colorstart[1], part.life);
	col[idx*4+2]=INTERP(psys->colorend[2], psys->colorstart[2], part.life);
	col[idx*4+3]=INTERP(psys->colorend[3], psys->colorstart[3], part.life);
#else
    glColor4f(INTERP(psys->colorend[0], psys->colorstart[0], part.life),
        INTERP(psys->colorend[1], psys->colorstart[1], part.life),
        INTERP(psys->colorend[2], psys->colorstart[2], part.life),
        INTERP(psys->colorend[3], psys->colorstart[3], part.life));
#endif

    vert = part.pos - pushx2 - pushy2;
#ifdef HAVE_GLES
	tex[idx*2+0]=0; tex[idx*2+1]=0;
	vtx[idx++]=vert;
#else
    glTexCoord2i(0,0);
    glVertex3fv(vert);
#endif
    vert = part.pos + pushx2 - pushy2;
#ifdef HAVE_GLES
	memcpy(&col[idx], &col[idx-1], sizeof(GLfloat)*4);
	tex[idx*2+0]=1; tex[idx*2+1]=0;
	vtx[idx++]=vert;
#else
    glTexCoord2i(1,0);
    glVertex3fv(vert);
#endif
    vert = part.pos + pushx2 + pushy2;
#ifdef HAVE_GLES
	memcpy(&col[idx], &col[idx-1], sizeof(GLfloat)*4);
	tex[idx*2+0]=1; tex[idx*2+1]=1;
	vtx[idx++]=vert;
#else
    glTexCoord2i(1,1);
    glVertex3fv(vert);
#endif
    vert = part.pos - pushx2 + pushy2;
#ifdef HAVE_GLES
	memcpy(&col[idx], &col[idx-1], sizeof(GLfloat)*4);
	tex[idx*2+0]=0; tex[idx*2+1]=1;
	vtx[idx++]=vert;
	// split a Quad in 2 triangles
	indices[indice++]=idx-4;
	indices[indice++]=idx-3;
	indices[indice++]=idx-2;
	
	indices[indice++]=idx-2;
	indices[indice++]=idx-1;
	indices[indice++]=idx-4;
#else
    glTexCoord2i(0,1);
    glVertex3fv(vert);
#endif
  }
#ifdef HAVE_GLES
  glDrawElements(GL_TRIANGLES, indice, GL_UNSIGNED_SHORT, indices);
  if (!glCol)	glDisableClientState(GL_COLOR_ARRAY);
  if (!glTex)	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  if (!glVtx)	glDisableClientState(GL_VERTEX_ARRAY);
#else
  glEnd();
#endif
  
  if (!psys->tex) glEnable(GL_TEXTURE_2D);
}

void PSSRender::drawModel(PModel &model, PSSEffect &ssEffect, PSSTexture &ssTexture)
{
#ifdef HAVE_GLES
  GLboolean glTex = glIsEnabled(GL_TEXTURE_COORD_ARRAY);
  GLboolean glVtx = glIsEnabled(GL_VERTEX_ARRAY);
  GLboolean glNrm = glIsEnabled(GL_NORMAL_ARRAY);
//  if (!glCol)	glEnableClientState(GL_COLOR_ARRAY);
  if (!glTex)	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  if (!glVtx)	glEnableClientState(GL_VERTEX_ARRAY);
  if (!glNrm)	glEnableClientState(GL_NORMAL_ARRAY);
  GLshort tex[1024*2];
  vec3f vtx[1024];
  vec3f nrm[1024];
  int idx=0;
  glVertexPointer(3, GL_FLOAT, 0, vtx);
  glTexCoordPointer(2, GL_FLOAT, 0, tex);
  glNormalPointer(GL_FLOAT, 0, nrm);
#endif
  for (std::vector<PMesh>::iterator mesh = model.mesh.begin();
    mesh != model.mesh.end();
    mesh++) {
    if (!mesh->effect)
      mesh->effect = ssEffect.loadEffect(mesh->fxname);

    int numPasses = 0;
    if (mesh->effect->renderBegin(&numPasses, ssTexture)) {
      for (int i=0; i<numPasses; i++) {
        mesh->effect->renderPass(i);
#ifdef HAVE_GLES
		idx=0;
#else
        glBegin(GL_TRIANGLES);
#endif
        for (unsigned int f=0; f<mesh->face.size(); f++) {
          //glNormal3fv(mesh->face[f].facenormal);
#ifdef HAVE_GLES
		  memcpy(&nrm[idx], mesh->norm[mesh->face[f].nr[0]], sizeof(vec3f));
		  memcpy(&tex[idx*2], mesh->texco[mesh->face[f].tc[0]], sizeof(GLfloat)*2);
		  memcpy(&vtx[idx], mesh->vert[mesh->face[f].vt[0]], sizeof(vec3f));
		  idx++;
		  memcpy(&nrm[idx], mesh->norm[mesh->face[f].nr[1]], sizeof(vec3f));
		  memcpy(&tex[idx*2], mesh->texco[mesh->face[f].tc[1]], sizeof(GLfloat)*2);
		  memcpy(&vtx[idx], mesh->vert[mesh->face[f].vt[1]], sizeof(vec3f));
		  idx++;
		  memcpy(&nrm[idx], mesh->norm[mesh->face[f].nr[2]], sizeof(vec3f));
		  memcpy(&tex[idx*2], mesh->texco[mesh->face[f].tc[2]], sizeof(GLfloat)*2);
		  memcpy(&vtx[idx], mesh->vert[mesh->face[f].vt[2]], sizeof(vec3f));
		  idx++;
#else
          glNormal3fv(mesh->norm[mesh->face[f].nr[0]]);
          glTexCoord2fv(mesh->texco[mesh->face[f].tc[0]]);
          glVertex3fv(mesh->vert[mesh->face[f].vt[0]]);

          glNormal3fv(mesh->norm[mesh->face[f].nr[1]]);
          glTexCoord2fv(mesh->texco[mesh->face[f].tc[1]]);
          glVertex3fv(mesh->vert[mesh->face[f].vt[1]]);

          glNormal3fv(mesh->norm[mesh->face[f].nr[2]]);
          glTexCoord2fv(mesh->texco[mesh->face[f].tc[2]]);
          glVertex3fv(mesh->vert[mesh->face[f].vt[2]]);
#endif
        }
#ifdef HAVE_GLES
		glDrawArrays(GL_TRIANGLES, 0, idx);
#else
        glEnd();
#endif
      }
      mesh->effect->renderEnd();
    }
  }
#ifdef HAVE_GLES
//  if (!glCol)	glDisableClientState(GL_COLOR_ARRAY);
  if (!glTex)	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  if (!glVtx)	glDisableClientState(GL_VERTEX_ARRAY);
  if (!glNrm)	glDisableClientState(GL_NORMAL_ARRAY);
#endif
}

void PSSRender::drawText(const std::string &text, uint32 flags)
{
  const float font_aspect = 0.6f;
  
  glPushMatrix();
  
  if (flags & PTEXT_VTA_CENTER)
    glTranslatef(0.0f, -0.5f, 0.0f);
  else if (flags & PTEXT_VTA_TOP)
    glTranslatef(0.0f, -1.0f, 0.0f);
  
  if (flags & PTEXT_HZA_CENTER)
    glTranslatef(-((float)text.length()) * 0.5f * font_aspect, 0.0f, 0.0f);
  else if (flags & PTEXT_HZA_RIGHT)
    glTranslatef(-((float)text.length()) * font_aspect, 0.0f, 0.0f);
  
#ifdef HAVE_GLES
//  GLboolean glCol = glIsEnabled(GL_COLOR_ARRAY);
  GLboolean glTex = glIsEnabled(GL_TEXTURE_COORD_ARRAY);
  GLboolean glVtx = glIsEnabled(GL_VERTEX_ARRAY);
//  if (!glCol)	glEnableClientState(GL_COLOR_ARRAY);
  if (!glTex)	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  if (!glVtx)	glEnableClientState(GL_VERTEX_ARRAY);
  GLfloat tex[1024*2];
  vec2f vtx[1024];
  int idx=0;
  GLfloat vtx_x = 0.0f;
  GLshort indices[1024*6/4];
  int indice=0;
  glVertexPointer(2, GL_FLOAT, 0, vtx);
  glTexCoordPointer(2, GL_FLOAT, 0, tex);
#endif
  for (std::string::const_iterator c = text.begin(); c != text.end(); c++) {
    float tx = ((float)(*c % 16) + 0.5f - font_aspect*0.5f) / 16.0f, addx = font_aspect / 16.0f;
    float ty = (*c / 16) / 16.0f, addy = 1 / 16.0f;
#ifdef HAVE_GLES
	tex[idx*2+0]=tx; tex[idx*2+1]=ty;
	vtx[idx][0] = vtx_x; vtx[idx][1] = 0.0f;
	idx++;
	tex[idx*2+0]=tx+addx; tex[idx*2+1]=ty;
	vtx[idx][0] = vtx_x+font_aspect; vtx[idx][1] = 0.0f;
	idx++;
	tex[idx*2+0]=tx+addx; tex[idx*2+1]=ty+addy;
	vtx[idx][0] = vtx_x+font_aspect; vtx[idx][1] = 1.0f;
	idx++;
	tex[idx*2+0]=tx; tex[idx*2+1]=ty+addy;
	vtx[idx][0] = vtx_x; vtx[idx][1] = 1.0f;
	idx++;
	// split a Quad in 2 triangles
	indices[indice++]=idx-4;
	indices[indice++]=idx-3;
	indices[indice++]=idx-2;
	
	indices[indice++]=idx-2;
	indices[indice++]=idx-1;
	indices[indice++]=idx-4;
	// translate
	vtx_x+=font_aspect;
#else
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(tx,ty);
    glVertex2f(0.0f,0.0f);
    glTexCoord2f(tx+addx,ty);
    glVertex2f(font_aspect,0.0f);
    glTexCoord2f(tx,ty+addy);
    glVertex2f(0.0f,1.0f);
    glTexCoord2f(tx+addx,ty+addy);
    glVertex2f(font_aspect,1.0f);
    glEnd();
    glTranslatef(font_aspect,0,0);
#endif
  }
  
#ifdef HAVE_GLES
  glDrawElements(GL_TRIANGLES, indice, GL_UNSIGNED_SHORT, indices);
//  if (!glCol)	glDisableClientState(GL_COLOR_ARRAY);
  if (!glTex)	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  if (!glVtx)	glDisableClientState(GL_VERTEX_ARRAY);
#endif
  glPopMatrix();
}

vec2f PSSRender::getTextDims(const std::string &text)
{
  const float font_aspect = 0.6f;
  
  return vec2f((float)text.length() * font_aspect, 1.0f);
}


void PParticleSystem::addParticle(const vec3f &pos, const vec3f &linvel)
{
  part.push_back(PParticle_s());
  part.back().pos = pos;
  part.back().linvel = linvel;
  part.back().life = 1.0;

  float ang = randm11 * PI;
  part.back().orix = vec2f(cos(ang),sin(ang));
  part.back().oriy = vec2f(-sin(ang),cos(ang));
}


void PParticleSystem::tick(float delta)
{
  float decr = delta * decay;

  // update life and delete dead particles
  unsigned int j=0;
  for (unsigned int i=0; i<part.size(); i++) {
    part[j] = part[i];
    part[j].life -= decr;
    if (part[j].life > 0.0) j++;
  }
  part.resize(j);
  
  for (unsigned int i=0; i<part.size(); i++) {
    part[i].pos += part[i].linvel * delta;
  }
}




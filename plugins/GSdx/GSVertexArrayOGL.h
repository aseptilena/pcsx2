/*
 *	Copyright (C) 2011-2011 Gregory hainaut
 *	Copyright (C) 2007-2009 Gabest
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#pragma once

struct GSInputLayoutOGL {
	GLuint  index;
	GLint   size;
	GLenum  type;
	GLboolean normalize;
	GLsizei stride;
	const GLvoid* offset;
};

class GSBufferOGL {
	const size_t m_stride;
	size_t m_start;
	size_t m_count;
	size_t m_limit;
	const GLenum m_target;
	GLuint m_buffer;
	const bool m_sub_data_config;

	public: 
	GSBufferOGL(GLenum target, size_t stride) : 
		m_stride(stride)
		, m_start(0)
		, m_count(0)
		, m_limit(0)
		, m_target(target)
		, m_sub_data_config(theApp.GetConfig("ogl_vertex_subdata", 1) != 0)
	{
		gl_GenBuffers(1, &m_buffer);
		// Opengl works best with 1-4MB buffer.
		// Warning m_limit is the number of object (not the size in Bytes)
		m_limit = 2 * 1024 * 1024 / m_stride;
		//m_limit = 512 * 1024 * m_stride;
	}

	~GSBufferOGL() { gl_DeleteBuffers(1, &m_buffer); }

	void allocate() { allocate(m_limit); }

	void allocate(size_t new_limit)
	{
		m_start = 0;
		m_limit = new_limit;
		gl_BufferData(m_target,  m_limit * m_stride, NULL, GL_STREAM_DRAW);
	}

	void bind()
	{
		gl_BindBuffer(m_target, m_buffer);
	}

	void subdata_upload(const void* src, uint32 count)
	{
		m_count = count;

		// Current GPU buffer is really too small need to allocate a new one
		if (m_count > m_limit) {
			//fprintf(stderr, "Allocate a new buffer\n %d", m_stride);
			allocate(std::max<int>(m_count * 3 / 2, m_limit));

		} else if (m_count > (m_limit - m_start) ) {
			//fprintf(stderr, "Orphan the buffer %d\n", m_stride);

			// Not enough left free room. Just go back at the beginning
			m_start = 0;
			// Orphan the buffer to avoid synchronization
			allocate(m_limit);
		}

		gl_BufferSubData(m_target,  m_stride * m_start,  m_stride * m_count, src);
	}

	void map_upload(const void* src, uint32 count)
	{
		void* dst;
		if (Map(&dst, count)) {
			// FIXME which one to use. Note dst doesn't have any aligment guarantee
			// because it depends of the offset
			//GSVector4i::storent(dst, src, m_count * m_stride);
			memcpy(dst, src, m_stride*m_count);
			Unmap();
		}
	}

#ifdef ENABLE_GLES
	void upload(const void* src, uint32 count, uint32 basevertex = 0)
#else
	void upload(const void* src, uint32 count)
#endif
	{
#ifdef ENABLE_GLES
		// Emulate gl_DrawElementsBaseVertex... Maybe GLES 4 you know!
		if (basevertex) {
			uint32* data = (uint32*) src;
			for (uint32 i = 0; i < count; i++) {
				data[i] += basevertex;
			}
		}
#endif
		if (m_sub_data_config) {
			subdata_upload(src, count);
		} else {
			map_upload(src, count);
		}
	}

	bool Map(void** pointer, uint32 count ) {
		m_count = count;

		// Note: For an explanation of the map flag
		// see http://www.opengl.org/wiki/Buffer_Object_Streaming
		uint32 map_flags = GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT;

		// Current GPU buffer is really too small need to allocate a new one
		if (m_count > m_limit) {
			allocate(std::max<int>(m_count * 3 / 2, m_limit));

		} else if (m_count > (m_limit - m_start) ) {
			// Not enough left free room. Just go back at the beginning
			m_start = 0;

			// Tell the driver that it can orphan previous buffer and restart from a scratch buffer.
			// Technically the buffer will not be accessible by the application anymore but the
			// GL will effectively remove it when draws call are finised.
			map_flags |= GL_MAP_INVALIDATE_BUFFER_BIT;
		} else {
			// Tell the driver that it doesn't need to contain any valid buffer data, and that you promise to write the entire range you map
			map_flags |= GL_MAP_INVALIDATE_RANGE_BIT;
		}

		// Upload the data to the buffer
		*pointer = (uint8*) gl_MapBufferRange(m_target, m_stride*m_start, m_stride*m_count, map_flags);

		return true;
	}

	void Unmap() { gl_UnmapBuffer(m_target); }

	void EndScene()
	{
		m_start += m_count;
		m_count = 0;
	}

	void Draw(GLenum mode)
	{
		glDrawArrays(mode, m_start, m_count);
	}

	void Draw(GLenum mode, GLint basevertex)
	{
#ifdef ENABLE_GLES
		glDrawElements(mode, m_count, GL_UNSIGNED_INT, (void*)(m_start * m_stride));
#else
		gl_DrawElementsBaseVertex(mode, m_count, GL_UNSIGNED_INT, (void*)(m_start * m_stride), basevertex);
#endif
	}

	void Draw(GLenum mode, GLint basevertex, int offset, int count)
	{
#ifdef ENABLE_GLES
		glDrawElements(mode, count, GL_UNSIGNED_INT, (void*)((m_start + offset) * m_stride));
#else
		gl_DrawElementsBaseVertex(mode, count, GL_UNSIGNED_INT, (void*)((m_start + offset) * m_stride), basevertex);
#endif
	}

	size_t GetStart() { return m_start; }

	void debug() 
	{
		fprintf(stderr, "data buffer: start %d, count %d\n", m_start, m_count);
	}

};

class GSVertexBufferStateOGL {
	GSBufferOGL *m_vb;
	GSBufferOGL *m_ib;

	GLuint m_va;
	GLenum m_topology;

public:
	GSVertexBufferStateOGL(size_t stride, GSInputLayoutOGL* layout, uint32 layout_nbr)
	{
		gl_GenVertexArrays(1, &m_va);

		m_vb = new GSBufferOGL(GL_ARRAY_BUFFER, stride);
		m_ib = new GSBufferOGL(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32));

		bind();
		// Note: index array are part of the VA state so it need to be bind only once.
		m_ib->bind();

		m_vb->allocate();
		m_ib->allocate();
		set_internal_format(layout, layout_nbr);
	}

	void bind()
	{
		gl_BindVertexArray(m_va);
		m_vb->bind();
	}

	void set_internal_format(GSInputLayoutOGL* layout, uint32 layout_nbr)
	{
		for (uint32 i = 0; i < layout_nbr; i++) {
			// Note this function need both a vertex array object and a GL_ARRAY_BUFFER buffer
			gl_EnableVertexAttribArray(layout[i].index);
			switch (layout[i].type) {
				case GL_UNSIGNED_SHORT:
				case GL_UNSIGNED_INT:
					// Rule: when shader use integral (not normalized) you must use gl_VertexAttribIPointer (note the extra I)
					gl_VertexAttribIPointer(layout[i].index, layout[i].size, layout[i].type, layout[i].stride, layout[i].offset);
					break;
				default:
					gl_VertexAttribPointer(layout[i].index, layout[i].size, layout[i].type, layout[i].normalize,  layout[i].stride, layout[i].offset);
					break;
			}
		}
	}

	void EndScene()
	{
		m_vb->EndScene();
		m_ib->EndScene();
	}

	void DrawPrimitive() { m_vb->Draw(m_topology); }

	void DrawIndexedPrimitive() { m_ib->Draw(m_topology, m_vb->GetStart() ); }

	void DrawIndexedPrimitive(int offset, int count) { m_ib->Draw(m_topology, m_vb->GetStart(), offset, count ); }

	void SetTopology(GLenum topology) { m_topology = topology; }

	void UploadVB(const void* vertices, size_t count) { m_vb->upload(vertices, count); }

	void UploadIB(const void* index, size_t count) {
#ifdef ENABLE_GLES
		m_ib->upload(index, count, m_vb->GetStart());
#else
		m_ib->upload(index, count);
#endif
	}

	bool MapVB(void **pointer, size_t count) { return m_vb->Map(pointer, count); }

	void UnmapVB() { m_vb->Unmap(); }

	~GSVertexBufferStateOGL()
	{
		gl_DeleteVertexArrays(1, &m_va);
		delete m_vb;
		delete m_ib;
	}

	void debug()
	{
		string topo;
		switch (m_topology) {
			case GL_POINTS:
				topo = "point";
				break;
			case GL_LINES:
				topo = "line";
				break;
			case GL_TRIANGLES: 
				topo = "triangle";
				break;
			case GL_TRIANGLE_STRIP:
				topo = "triangle strip";
				break;
		}
		m_vb->debug();
		m_ib->debug();
		fprintf(stderr, "primitives of %s\n", topo.c_str());

	}
};

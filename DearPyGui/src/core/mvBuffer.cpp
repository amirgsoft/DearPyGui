#include "mvBuffer.h"

namespace Marvel {

	void intialize_mvBuffer(mvBuffer* a, long length)
	{
		a->length = length;
		a->data = new float[length];
		for (int i = 0; i < length; i++)
			a->data[i] = 1.0f;
	}

	void deallocate_mvBuffer(mvBuffer* a)
	{
		delete[] a->data;
		a->data = nullptr;
	}

	int PymvBuffer_init(PymvBuffer* self, PyObject* args, PyObject* kwds)
	{
		// init may have already been called
		if (self->arr.data != NULL)
			deallocate_mvBuffer(&self->arr);

		int length = 0;
		static char* kwlist[] = { "length", NULL };
		if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i", kwlist, &length))
			return -1;

		if (length < 0)
			length = 0;

		intialize_mvBuffer(&self->arr, length);

		return 0;
	}

	void PymvBuffer_dealloc(PymvBuffer* self)
	{
		deallocate_mvBuffer(&self->arr);
		Py_TYPE(self)->tp_free((PyObject*)self);
	}

	char* stringify(mvBuffer* a, int nmax) 
	{
		char* output = (char*)malloc(nmax * 20);
		int pos = sprintf(&output[0], "[");

		for (int k = 0; k < a->length && k < nmax; k++) {
			pos += sprintf(&output[pos], " %f", a->data[k]);
		}
		if (a->length > nmax)
			pos += sprintf(&output[pos], "...");
		sprintf(&output[pos], " ]");
		return output;
	}

	PyObject* PymvBuffer_str(PymvBuffer* self)
	{
		char* s = stringify(&self->arr, 10);
		PyObject* ret = PyUnicode_FromString(s);
		free(s);
		return ret;
	}

	int PymvBuffer_getbuffer(PyObject* obj, Py_buffer* view, int flags)
	{
		if (view == NULL) {
			PyErr_SetString(PyExc_ValueError, "NULL view in getbuffer");
			return -1;
		}

		PymvBuffer* self = (PymvBuffer*)obj;
		view->obj = (PyObject*)self;
		view->buf = (void*)self->arr.data;
		view->len = self->arr.length * sizeof(float);
		view->readonly = 0;
		view->itemsize = sizeof(float);
		//view->format = (char*)(PyBUF_CONTIG_RO | PyBUF_FORMAT);  // float
		view->format = "f";  // float
		view->ndim = 1;
		view->shape = (Py_ssize_t*)&self->arr.length;  // length-1 sequence of dimensions
		view->strides = &view->itemsize;  // for the simple case we can do this
		view->suboffsets = NULL;
		view->internal = NULL;

		Py_INCREF(self);  // need to increase the reference count
		return 0;
	}

	Py_ssize_t PymvBuffer_getLength(PyObject* obj)
	{
		PymvBuffer* self = (PymvBuffer*)obj;
		return self->arr.length;
	}

	PyObject* PymvBuffer_getItem(PyObject* obj, Py_ssize_t index)
	{
		PymvBuffer* self = (PymvBuffer*)obj;
		return Py_BuildValue("f", self->arr.data[index]);
	}

	int PymvBuffer_setItem(PyObject* obj, Py_ssize_t index, PyObject* value)
	{
		PymvBuffer* self = (PymvBuffer*)obj;
		self->arr.data[index] = (float)PyFloat_AsDouble(value);
		return 0;
	}
}
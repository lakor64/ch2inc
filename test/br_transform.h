#pragma once

typedef unsigned short br_uint_16;
typedef unsigned short br_angle;
typedef unsigned char br_uint_8;
typedef float br_scalar;

typedef struct br_matrix34 {
	br_scalar m[4][3];
} br_matrix34;

typedef struct br_euler {
	br_angle a;
	br_angle b;
	br_angle c;
	br_uint_8 order;
} br_euler;

typedef struct br_vector3 {
	br_scalar v[3];
} br_vector3;

typedef struct br_quat {
	br_scalar x;
	br_scalar y;
	br_scalar z;
	br_scalar w;
} br_quat;

typedef struct br_transform {
						   
	/*
	 * Type of position
	 */
	br_uint_16 type;

	/*
	 * Union of the various means of describing a transform -
	 * these are explicity arrranged so that any exlicit transform
	 * will always be available as br_transform.t.translate
	 */
	union {
		/*
		 * Affine 3x4 matrix
		 */
		br_matrix34 mat;

		/*
		 * Euler angles and translation
		 */
		struct {
			br_euler e;
			br_scalar _pad[7];
			br_vector3 t;
		} euler;

		/*
		 * Unit quaternion and translation
		 */
		struct {
			br_quat q;
			br_scalar _pad[5];
			br_vector3 t;
		} quat;

		/*
		 * Lookat vector, up vector and translation
		 */
		struct {
			br_vector3 look;
			br_vector3 up;
			br_scalar _pad[3];
			br_vector3 t;
		} look_up;

		/*
		 * Just a translation
		 */
		struct {
			br_scalar _pad[9];
			br_vector3 t;
		} translate;
	} t;
} br_transform;

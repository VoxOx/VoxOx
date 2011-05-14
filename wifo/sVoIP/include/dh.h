 /*
	Zenbow Secure VoIP.
	Copyright (C) 2006 Everbee Networks 

	* This program is free software; you can redistribute it and/or
	* modify it under the terms of the GNU General Public License
	* as published by the Free Software Foundation; either version 2
	* of the License, or (at your option) any later version.

	* This program is distributed in the hope that it will be useful,
	* but WITHOUT ANY WARRANTY; without even the implied warranty of
	* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	* GNU General Public License for more details.

	* You should have received a copy of the GNU General Public License
	* along with this program; if not, write to the Free Software
	* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

	* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
	* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
	* COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
	* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
	* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
	* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
	* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
	* OF THE POSSIBILITY OF SUCH DAMAGE.

*/

/** Diffie hellman structure (from OpenSSL) */
typedef struct DH DH;

/**
* Initialize Diffie Hellman engine
*/
void	dh_init();

/**
* Create a DH structure containing default generator parameters
* (Oakley 5)
* @return Diffie hellman structure
*/
DH	*dh_create_default();

/**
* Generate a (or b) and compute g^a (or g^a) of Diffie Hellman
* @return g^a in hexadecimal format
*/
char	*dh_generate_key(DH *dh);

/**
* Compute g^(ab) using the g^b (or g^a) from the opposite side
* @param DH Diffie Hellman structure
* @param pub_key g^b (or g^a) from the opposite side in hexa format
* @param len Length of g^(ab)
* @return Master key (g^(ab)) in binary format
*/
unsigned char	*dh_compute_key(DH *dh, char *pub_key, int *len);

/**
* Free Diffie Hellman structure
* @param dh Diffie Hellman structure
*/
void	dh_free(DH *dh);

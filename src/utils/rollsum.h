/*
**
** Copyright (c) 2011, Stephane Sezer
** All rights reserved.
** 
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of Stephane Sezer nor the names of its contributors
**       may be used to endorse or promote products derived from this software
**       without specific prior written permission.
** 
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL Stephane Sezer BE LIABLE FOR ANY DIRECT,
** INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
** LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
** ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
*/

#ifndef ROLLSUM_H_
# define ROLLSUM_H_

# define ROLLSUM_CHECKBITS          13
# define ROLLSUM_CHECKMASK          ((1 << ROLLSUM_CHECKBITS) - 1)
# define ROLLSUM_CHECKAVGCHUNKSIZE  (1 << ROLLSUM_CHECKBITS)
# define ROLLSUM_BOUNDVAL           0xa8
# define ROLLSUM_CHAROFFSET         31
# define ROLLSUM_ROLLWINDOW         48
# define ROLLSUM_MINSIZE            512
# define ROLLSUM_MAXSIZE            (ROLLSUM_CHECKAVGCHUNKSIZE * 2)

# define rollsumMinima(Sum) ((((Sum)->s1) & ROLLSUM_CHECKMASK) == ROLLSUM_BOUNDVAL)

# define rollsumRotate(Sum, Out, In)                                    \
do                                                                      \
{                                                                       \
  (Sum)->s1 += (unsigned char)(In) - (unsigned char)(Out);                                            \
  (Sum)->s2 += (Sum)->s1 - (Sum)->window_count * (((unsigned char)(Out)) + ROLLSUM_CHAROFFSET); \
} while (0)

struct rollsum
{
  int             count;
  unsigned char   window[ROLLSUM_ROLLWINDOW];
  unsigned int    window_count;
  unsigned int    window_cursor;
  unsigned short  s1;
  unsigned short  s2;
};

static inline void rollsum_init(struct rollsum *rs)
{
  memset(rs, 0, sizeof (*rs));
}

static inline int rollsum_roll(struct rollsum *rs, unsigned char c)
{
  int reached_boundary = 0;

  ++rs->count;

  int last_i;
  if (rs->window_cursor == 0)
    last_i = ROLLSUM_ROLLWINDOW - 1;
  else
    last_i = rs->window_cursor-1;

  rollsumRotate(rs, rs->window[last_i], c);

  rs->window[rs->window_cursor++] = c;
  rs->window_count++;

  if (rs->window_cursor == ROLLSUM_ROLLWINDOW)
    rs->window_cursor = 0;

  if (rs->count == ROLLSUM_MAXSIZE)
  {
    reached_boundary = 1;
  }
  else if (rs->count > ROLLSUM_MINSIZE)
  {
    if (rs->window_count >= ROLLSUM_ROLLWINDOW && rollsumMinima(rs))
      reached_boundary = 1;
  }

  /* no need to clean up, buffer is emptied each time
    if (reached_boundary == 1) {
    rs->window_cursor = 0;
    rs->window_count = 0;
    bzero(rs->window, sizeof(rs->window));
  }
  */
  return reached_boundary;
}

#endif /* !ROLLSUM_H_ */

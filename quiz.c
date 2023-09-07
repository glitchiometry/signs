#include "SDL.h"
#include "SDL_video.h"
#include "basics.h"

#define BATCH_LEN 5

array_int letters_score;
array_int letters_total;
array_int letters_nsamples;
array_char letters_;
array_int numbers_;
array_int numbers_score;
array_int numbers_total;
array_int numbers_nsamples;
array_char letters_covered;
int covered_letters[BATCH_LEN];
array_char numbers_covered;
int covered_numbers[BATCH_LEN];

void init_arrays();
void print_session_summary(FILE *ofile);
SDL_Texture* update_SDL_texture(char *fname);
void reset_state_vars();
void home_state_subroutine(const Uint8 *kb_state, SDL_Event *e);
void menu_state_subroutine(const Uint8 *kb_state, SDL_Event *e);
void letter_state_subroutine(const Uint8 *kb_state, SDL_Event *e);
void number_state_subroutine(const Uint8 *kb_state, SDL_Event *e);
void render_SDL_image();
char parse_number_entry(char *number_buf, int len, int *entry);

char mode = 'h';
char qa_mode = 0;
char buf[256];
char number_buf[64];
int number_buf_cnt;
int batch_count;
int i_sym;

SDL_Texture *tex;
SDL_Renderer *rndrr;

int main(int argc, char *argv[])
{
	init_arrays();
	int N_VideoDrivers = SDL_GetNumVideoDrivers();
	if (N_VideoDrivers > 0) {}
	else
	{
		printf("Error: N_VideoDrivers = %d\n", N_VideoDrivers);
		exit(EXIT_FAILURE);
	}
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO);
	//IMG_Init(IMG_INIT_PNG);
	const char *default_vdriver = SDL_GetVideoDriver(0);
	printf("Video driver: %s\n", default_vdriver);
	SDL_VideoInit(default_vdriver);
	SDL_Window* win = SDL_CreateWindow("ASL_quiz", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 960, 540, SDL_WINDOW_OPENGL);
	rndrr = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	tex = update_SDL_texture("menu/home.bmp");
	int num_keys = 0;
	const Uint8 *kb_state = SDL_GetKeyboardState(&num_keys);
	printf("Number of keys: %d\n", num_keys);
	while (1) 
	{
		SDL_Event e;
		if (SDL_WaitEvent(&e)) 
		{
			if (e.type == SDL_QUIT || mode == 'q') break;
			if (e.type == SDL_MOUSEBUTTONDOWN)
			{
				printf("Mode = %c\n", mode);
			}
			if (mode == 'h')
			{
				home_state_subroutine(kb_state, &e);
			}
			else if (mode == 'm')
			{
				menu_state_subroutine(kb_state, &e);
			}
			else if (mode == 'l')
			{
				letter_state_subroutine(kb_state, &e);
		
			}
			else if (mode == 'n')
			{
				number_state_subroutine(kb_state, &e);
			}
		}
		SDL_RenderClear(rndrr);
		SDL_RenderCopy(rndrr, tex, NULL, NULL);
		SDL_RenderPresent(rndrr);
	}
	SDL_DestroyRenderer(rndrr);
	SDL_DestroyWindow(win);
	//IMG_Quit();
	SDL_VideoQuit();
	SDL_Quit();
	if (argc > 1)
	{
		FILE *ofile = fopen(argv[1], "w");
		print_session_summary(ofile);
		fclose(ofile);
	}
	else
	{
		print_session_summary(stdout);
	}
	free_array_char(&letters_);
	free_array_int(&numbers_);
	free_array_int(&letters_score);
	free_array_int(&letters_total);
	free_array_char(&letters_covered);
	free_array_int(&numbers_score);
	free_array_int(&numbers_total);
	free_array_char(&numbers_covered);
	return 0;
}

void render_SDL_image()
{
	SDL_RenderClear(rndrr);
	SDL_RenderCopy(rndrr, tex, NULL, NULL);
	SDL_RenderPresent(rndrr);
}

SDL_Texture *update_SDL_texture(char *fname)
{
	//	SDL_Surface *surf = IMG_Load(fname);
	SDL_Surface *surf = SDL_LoadBMP(fname);
	SDL_Texture *tex_ = SDL_CreateTextureFromSurface(rndrr, surf);
	SDL_FreeSurface(surf);
	return tex_;
}

void img_name_letter(char l, int sample_index, char *name)
{
	sprintf(name, "letters/_%c_/%c_%d.bmp", l, l, sample_index);
	printf("img: %s\n", name);
}

void img_name_number(int n, int sample_index, char *name)
{
	sprintf(name, "numbers/%d_%d.bmp", n, sample_index);
}

void init_arrays()
{
	array_int_init(&letters_score, 1);
	array_int_init(&letters_total, 1);
	array_char_init(&letters_covered, 1);
	array_char_init(&letters_, 1);
	array_int_init(&letters_nsamples, 1);
	array_int_init(&numbers_, 1);
	array_int_init(&numbers_score, 1);
	array_int_init(&numbers_total, 1);
	array_char_init(&numbers_covered, 1);
	array_int_init(&numbers_nsamples, 1);
	FILE *lfile = fopen("letters.dat", "r");
	if (lfile != NULL)
	{
		char letter_;
		int nsamples;
		while (fscanf(lfile, "%c %d\n", &letter_, &nsamples) != EOF)
		{
			printf("Adding %c\n", letter_);
			add2array_char(&letters_, letter_);
			add2array_int(&letters_score, 0);
			add2array_int(&letters_total, 0);
			add2array_char(&letters_covered, 0);
			add2array_int(&letters_nsamples, nsamples);
		}
		fclose(lfile);
	}
	FILE *nfile = fopen("numbers.dat", "r");
	if (nfile != NULL)
	{
		int number_, nsamples;
		while (fscanf(nfile, "%d %d", &number_, &nsamples) != EOF)
		{
			add2array_int(&numbers_, number_);
			add2array_int(&numbers_score, 0);
			add2array_int(&numbers_total, 0);
			add2array_char(&numbers_covered, 0);
			add2array_int(&numbers_nsamples, nsamples);
		}
		fclose(nfile);
	}
}

void print_session_summary(FILE *ofile)
{
	if (ofile != NULL)
	{
		fprintf(ofile, "Letters score:\n");
		for (int i = 0; i < letters_.len; i++)
		{
			fprintf(ofile, " %d", letters_score.e[i]);
		}
		fprintf(ofile, "\n");
		for (int i = 0; i < letters_.len; i++)
		{
			fprintf(ofile, " %d", letters_total.e[i]);
		}
		fprintf(ofile, "\nNumbers score:\n");
		for (int i = 0; i < numbers_.len; i++)
		{
			fprintf(ofile, " %d", numbers_score.e[i]);
		}
		fprintf(ofile, "\n");
		for (int i = 0; i < numbers_.len; i++)
		{
			fprintf(ofile, " %d", numbers_total.e[i]);
		}
		if (ofile == stdout) fprintf(ofile, "\n");
		fclose(ofile);
	}
}

void reset_state_vars()
{
	mode = 'h';
	qa_mode = 0;
	number_buf[0] = '\0';
	number_buf_cnt = 0;
	batch_count = 0;
}

void home_state_subroutine(const Uint8 *kb_state, SDL_Event *e)
{
	if ((*e).type == SDL_KEYDOWN)
	{
		if (kb_state[SDL_SCANCODE_C])
		{
			mode = 'm';
			tex = update_SDL_texture("menu/menu_.bmp");
		}
		if (kb_state[SDL_SCANCODE_Q])
		{
			mode = 'q';
		}
		if (kb_state[SDL_SCANCODE_A])
		{
			mode = 'a';
			tex = update_SDL_texture("menu/acknowledgements.bmp");
		}
	}
}

void menu_state_subroutine(const Uint8 *kb_state, SDL_Event *e)
{
	if ((*e).type == SDL_KEYDOWN)
	{
		if (kb_state[SDL_SCANCODE_L])
		{
			mode = 'l';
			batch_count = 0;
		}
		if (kb_state[SDL_SCANCODE_N])
		{
			mode = 'n';
			batch_count = 0;
		}
		if (kb_state[SDL_SCANCODE_Q])
		{
			mode = 'q';
		}
	}
}

void letter_state_subroutine(const Uint8 *kb_state, SDL_Event *e)
{
	if ((*e).type == SDL_KEYDOWN)
	{
		if (qa_mode == 0)
		{
			do
			{
				i_sym = rand() % letters_.len;
			} while (letters_covered.e[i_sym] == 1);
			int sample_index = rand() % letters_nsamples.e[i_sym];
			SDL_DestroyTexture(tex);
			img_name_letter(letters_.e[i_sym], sample_index, buf);
			tex = update_SDL_texture(buf);
			qa_mode = 1;
		}
		else if (qa_mode == 1)
		{
			// Check if the entered key matches the letter shown
			if ((*e).key.keysym.sym == letters_.e[i_sym])
			{
				printf("Correct!\n");
				letters_score.e[i_sym] += 1;
				qa_mode = 0;
				covered_letters[batch_count] = i_sym;
				letters_covered.e[i_sym] = 1;
				batch_count += 1;
				if (batch_count == BATCH_LEN)
				{
					mode = 'm';
					tex = update_SDL_texture("menu/menu_.bmp");
					batch_count = 0;
					for (int ii = 0; ii < BATCH_LEN; ii++)
					{
						letters_covered.e[covered_letters[ii]] = 0;
					}
				}
			}
			else
			{
				qa_mode = 2;
				char local_buf[256];
				sprintf(local_buf, "letters/_%c_/ans_%c.bmp", letters_.e[i_sym], letters_.e[i_sym]);
				tex = update_SDL_texture(local_buf);
			}
			letters_total.e[i_sym] += 1;
		}
		else if (qa_mode == 2)
		{
			// Maintain this mode to check the last answer
			if (kb_state[SDL_SCANCODE_C] == 1)
			{
				qa_mode = 0;
				covered_letters[batch_count] = i_sym;
				letters_covered.e[i_sym] = 1;
				batch_count += 1;
				if (batch_count == BATCH_LEN)
				{
					mode = 'm';
					tex = update_SDL_texture("menu/menu_.bmp");
					batch_count = 0;
					for (int ii = 0; ii < BATCH_LEN; ii++)
					{
						letters_covered.e[covered_letters[ii]] = 0;
					}
				}
			}
			
		}
	}
}

void number_state_subroutine(const Uint8 *kb_state, SDL_Event *e)
{
	if ((*e).type == SDL_KEYDOWN)
	{
		if (qa_mode == 0)
		{
			do
			{
				i_sym = rand() % numbers_.len;
			} while (numbers_covered.e[i_sym] == 1);
			SDL_DestroyTexture(tex);
			int sample_index = rand() % numbers_nsamples.e[i_sym];
			img_name_number(numbers_.e[i_sym], sample_index, buf);
			tex = update_SDL_texture(buf);
			qa_mode = 1;
		}
		else if (qa_mode == 1)
		{
			if (kb_state[SDL_SCANCODE_RETURN] == 1)
			{
				// Check if the entered key matches the letter shown
				// Map the input number
				int entry;
				char status = parse_number_entry(number_buf, number_buf_cnt, &entry);
				if (status) 
				{
					printf("Unable to interpret input %s\n", number_buf);
					number_buf_cnt = 0;
				}
				else
				{
					number_buf_cnt = 0;
					if (entry == numbers_.e[i_sym])
					{
						printf("Correct!\n");
						numbers_score.e[i_sym] += 1;
						qa_mode = 0;
						numbers_covered.e[i_sym] = 1;
						covered_numbers[batch_count] = i_sym;
						batch_count += 1;
						if (batch_count == BATCH_LEN)
						{
							tex = update_SDL_texture("menu/menu_.bmp");
							mode = 'm';
							batch_count = 0;
							for (int ii = 0; ii < BATCH_LEN; ii++)
							{
								numbers_covered.e[covered_numbers[ii]] = 0;
							}
						}
						//else number_state_subroutine(kb_state, e);
					}
					else
					{
						// Update image
						qa_mode = 2;
						char local_buf[256];
						sprintf(local_buf, "numbers/ans_%d.bmp", numbers_.e[i_sym]);
						tex = update_SDL_texture(local_buf);
						qa_mode = 2;
					}
					numbers_total.e[i_sym] += 1;
				}
				
			}
			else
			{
				number_buf[number_buf_cnt] = (*e).key.keysym.sym;
				printf("Read %c\n", number_buf[number_buf_cnt]);
				number_buf_cnt += 1;
			}
		}
		else if (qa_mode == 2)
		{
			if (kb_state[SDL_SCANCODE_C] == 1) 
			{
				qa_mode = 0;
				numbers_covered.e[i_sym] = 1;
				covered_numbers[batch_count] = i_sym;
				batch_count += 1;
				if (batch_count == BATCH_LEN)
				{
					mode = 'm';
					batch_count = 0;
					for (int ii = 0; ii < BATCH_LEN; ii++)
					{
						numbers_covered.e[covered_numbers[ii]] = 0;
					}
					tex = update_SDL_texture("menu/menu_.bmp");
				}
				//else number_state_subroutine(kb_state, e);
			}
		}
	}
}

char parse_number_entry(char *number_buf, int len, int *entry)
{
	int pow = 1;
	int i = len;
	(*entry) = 0;
	do
	{
		i -= 1;
		int digit = number_buf[i] - 48;
		if (-1 < digit && digit < 10) (*entry) += pow * digit;
		else
		{
			printf("Invalid character");
			return 1;
		}
		pow *= 10;
	} while (i > 0);
	return 0;
}


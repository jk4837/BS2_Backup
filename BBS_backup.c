#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <math.h>
void fix_file_name(unsigned char * const head);
int trans_file(const char* fn_i, const char* fn_o);
int trans_directory(const char* fn_dir, const char* fn_i, const char* backup_dir, const int rec_level, const char* fn_rec);
void trans_file_content(FILE* fp_i, FILE* fp_o);
int main()
{
	FILE *fd_rec;
	int rec_level = 3;
	unsigned char *si, *ei;
	char opt[300] = { "" }, dir[300] = { "" }, output_dir[300] = { "" }, full_path[300] = { "D:\\GoogleDrive\\Dropbox\\PROGRAM\\for_git\\BS2_Backup_Data\\gem\\brd\\NCTU_KendoTK\\.DIR" }, board[16] = { "" };
	
	do
	{
		printf("Please input .DIR file path: \n");
		scanf("%s",full_path);
		si = strstr(full_path,".DIR");
	}while(!si);
	
	if (full_path[0] == '\"')
		strncpy(dir, full_path + 1, strlen(full_path) - 2 - 4);
	else
		strncpy(dir, full_path, strlen(full_path) - 4);
	printf("%s\n", dir);
	ei = strchr(full_path, '\\');
	while (strchr(ei + 1, '\\'))
	{
		si = ei;
		ei = strchr(si + 1, '\\');
	}
	strncpy(board, si + 1, ei - si - 1);
	printf("Board = %s\n", board);
	
	printf("目錄檔\n0.不紀錄\n1.紀錄一層\n3.全部紀錄(適用精華區)\n請選擇 : ");
	scanf("%d",&rec_level);
	
	if (rec_level < 0)
		rec_level = 0;
	else if (rec_level >= 3)
		rec_level = INT_MAX;

	sprintf(output_dir, "BBS_backup\\%s\\", board);

	sprintf(opt, "mkdir \"%s\"", output_dir);
	system(opt);

	if (rec_level > 0) {
		sprintf(opt, "%sdirectory.txt", output_dir);
		fd_rec = fopen(opt, "w");
		if (!fd_rec)
		{
			printf("Writing record file failed, %s\npath : \"%s\"\n", strerror(errno), opt);
			system("pause");
			return -1;
		}
	}

	sprintf(output_dir, "%scontent\\", output_dir);
	trans_directory(dir, full_path, output_dir, rec_level, fd_rec);

	if (rec_level > 0)
		fclose(fd_rec);

	printf("Finish!\n");
	system("pause");
	return 0;
}

int trans_directory(const char* fn_dir, const char* fn_i, const char* backup_dir, const int rec_level, const char* fd_rec) {
	int i = 0, size, pad, result;
	unsigned char *si, *ei, *title, is_dir;
	unsigned char ID[80], name[50], date[9], title0[73 + 3] = { 0xa1,0xbb,' ' };
	unsigned char file_path[300] = { "7\\A1234567" }, prop[12];
	unsigned char *file = &(file_path[2]);
	FILE *fd_i;   // directory fi
	char full_path[300] = { "" }, opt[300] = { "" };

	fd_i = fopen(fn_i, "rb");
	if (!fd_i)
	{
		printf("Loading directory file failed, %s\npath : \"%s\"\n", strerror(errno), fn_i);
		return -1;
	}

	fseek(fd_i, 0, SEEK_END);
	size = ftell(fd_i);         // file size
	fseek(fd_i, 0, SEEK_SET);

	pad = (int)log10(size / 256) + 1;

	sprintf(opt, "mkdir \"%s\"", backup_dir);
	system(opt);

	while (++i)
	{
		fread(prop, sizeof(char), 12, fd_i);
		is_dir = (prop[6] == 0x01);
		if (fread(file, sizeof(char), 32, fd_i) <= 0)
			break;
		fread(ID, sizeof(char), 80, fd_i);
		si = strchr(ID, '.');    					// jk4837@ptt.cc F
		if (si)
			*(si + 1) = '\0';
		fread(name, sizeof(char), 50, fd_i);
		fseek(fd_i, 3, SEEK_CUR);           		// skip year
		fread(date, sizeof(char), 6, fd_i);
		fread(title0 + 3, sizeof(char), 73, fd_i);	//++(square symbol) when ! Re

		if (!strncmp(title0 + 3, "Re: ", 4)) {
			// in order to edit "Re: " to "Re "
			title = title0 + 4;   // e: ...
			title[0] = 'R';
			title[1] = 'e';

		}
		else {
			// not Re: adding square symbol
			if (is_dir)
				title0[1] = 0xbb;	// ◆
			else
				title0[1] = 0xba;
			title0[0] = 0xa1;
			title0[2] = ' ';
			title = title0;
		}

		if (file[0] == '@')
			file_path[0] = '@';
		else
			file_path[0] = file[7];

		sprintf(full_path, "%s%c\\%s", fn_dir, file_path[0], file);
		date[2] = '_';				// file name can't content '/'
		fix_file_name(title + 2);	// replace other invalid char with space
		memset(file_path, '\0', sizeof(file_path));
		// Here you can change the filename's formate of output file.
		if (strlen(date))
			sprintf(file_path, "%s\\%0*d  .  %s  .  %s", backup_dir, pad, i, date, title);
		else
			sprintf(file_path, "%s\\%0*d  .  %s", backup_dir, pad, i, title);

		if (!is_dir) {
			sprintf(file_path, "%s.txt", file_path);
			result = trans_file(full_path, file_path);
		}
		else
			result = trans_directory(fn_dir, full_path, file_path, rec_level - 1, fd_rec);	// only record at first time

		if (rec_level > 0) {
			fprintf(fd_rec, "%8s  %6d  %5s %-13s %-46s", full_path + strlen(full_path) - 10, i, date, ID, title);
			if (result)
				fprintf(fd_rec, "\t解析失敗!\n");
			else
				fprintf(fd_rec, "\n");
		}
	}
	fclose(fd_i);
}

// delete invalid char in file name
void fix_file_name(unsigned char * const head) {
	unsigned char *si, *last = head;
	for (si = head; *si != '\0'; si++)
	{
		if ((unsigned char)(*si) > 0x7f)     // Chinese
		{
			si++;
			if ((*(si) >= 0x40 && *(si) <= 0x7E) || (*(si) >= 0xA1 && *(si) <= 0xFE))
				last = si;
			else
			{
				*(si - 1) = ' ';
				*(si) = ' ';
			}
		}
		else if (*si == '\\' || *si == '/' || *si == ':' || *si == '*' || *si == '?' || *si == '\"' || *si == '<' || *si == '>' || *si == '|')
			*si = ' ';
		else
			last = si;
	}
	// make endding not space
	*(++last) = (unsigned int)'\0';
}

int trans_file(const char* fn_i, const char* fn_o) {
	FILE *fp_i, *fp_o;
	fp_i = fopen(fn_i, "r");
	if (!fp_i)
	{
		printf("Open file \"%s\" failed, %s\n", fn_i, strerror(errno));
		return -1;
	}

	fp_o = fopen(fn_o, "wb");
	if (!fp_o)
	{
		printf("Writing file \"%s\" failed, %s\n", fn_o, strerror(errno));
		fclose(fp_i);
		return -1;
	}

	trans_file_content(fp_i, fp_o);

	fclose(fp_i);
	fclose(fp_o);
	return 0;
}

void trans_file_content(FILE* fp_i, FILE* fp_o)
{
	unsigned char c;
	c = fgetc(fp_i);
	while (c != (unsigned char)EOF)
	{
		if (c == 0x1b)                 // *
		{
			c = fgetc(fp_i);
			if (c == 0x5b)             // [
			{
				while (c != 0x6d)      // m
					c = fgetc(fp_i);
			}
			if (c == 0x2a)             // push content control char
				c = fgetc(fp_i);
		}
		else if (c == 0x0a)
		{
			fputc(0x0d, fp_o);
			fputc(0x0a, fp_o);
		}
		else
		{
			fputc(c, fp_o);
			if ((unsigned char)c>0x7f)
			{
				c = fgetc(fp_i);
				fputc(c, fp_o);
			}
		}
		c = fgetc(fp_i);
	}
	fclose(fp_o);
	fclose(fp_i);
}

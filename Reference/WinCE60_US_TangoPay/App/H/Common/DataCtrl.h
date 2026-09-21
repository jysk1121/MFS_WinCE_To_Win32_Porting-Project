
// read_value, save_value 의 패러메터 값 최대 사이즈 
#define MAX_FILE_NAME_SIZE 20
#define MAX_ITEM_NAME_SIZE 20
#define MAX_VALUE_SIZE     1024

struct CONFFILE{
	char name[1024];
	char value[1024];
	struct CONFFILE *next;

	CONFFILE() : name(), value()
	{
	}
};
typedef struct CONFFILE CONF; //typedef문 쓸때만 메모리 사용하기 위해
typedef CONF *LINK;


// Function Prototypes
LINK add_to_section(char *name, char *value, LINK first );			//queue put
LINK find_key (char mode, char *Key, LINK first);
LINK find_section (char *Section, LINK first);
LINK read_list_file(char *filename,LINK first);						// 파일에서 내용을 읽어서 구조체에 넣는것
LINK add_to_list(char *name,char *value, LINK first);				// 링크 리스트 추가하기
int change_list(char *name,char *value,LINK first);					// 바꾸기 바꿀것이 있으면 1 없으면 -1 반환
char *get_list_value(char *name,LINK first);						// 읽어 오기
void write_list_file(char *filename,LINK first);					// 파일로 쓰기
LINK convert_list(char *filename,LINK first);
void delete_list(char *name,LINK first);							// 리스트 지우기
void free_memory_list(LINK first);									// 메모리 할당 제거
int align_printf(char *me, int me_len, char *instr, int instr_len, int align);
// end Prototypes


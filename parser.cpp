#include "parser.h"

XMLNode* XMLNodeNew(XMLNode* parent)
{
    XMLNode* node = (XMLNode*) malloc(sizeof(XMLNode));
    node->parent = parent;
    node->tag = nullptr;
    node->innerText = nullptr;
    XMLAttributeListInit(&node->attributes);
    return node;
}

void XMLAttributeFree(XMLAttribute* attr)
{
    free(attr->key);
    free(attr->value);
}

void XMLNodeFree(XMLNode* node)
{
    if(node->tag)
        free(node->tag);
    if(node->innerText)
        free(node->innerText);
    for(int i=0; i < node->attributes.size; i++)
        XMLAttributeFree(&node->attributes.data[i]);
}

void XMLAttributeListAdd(XMLAttributeList* list, XMLAttribute* attr)
{
    while(list->size >= list->heap_size) 
    {
        list->heap_size *= 2;
        list->data =  (XMLAttribute*) realloc(list->data, sizeof(XMLAttribute) * list->heap_size);
    }
    list->data[list->size++] = *attr;
}

void XMLAttributeListInit(XMLAttributeList* list)
{
    list->heap_size = 1; //number of elements we can put in
    list->size = 0; //number of elements we have.
    list->data = (XMLAttribute*) malloc(sizeof(XMLAttribute) * list->heap_size);
}

int XMLDocumentLoad(XMLDocument* doc, const char* path)
{
    FILE* file = fopen(path, "r");
    if(!file) 
    {
        std::cerr << "could not load file." << path << std::endl;
        return false;
    }
    fseek(file,0,SEEK_END); //total nums of chars
    int size = ftell(file);
    // std::cout << size;
    fseek(file, 0, SEEK_SET); //setting pos indicator to start of file
    char* buff = (char*)malloc(sizeof(char) * size+1);// +1 for extra null char.
    // char* buff = (char*) new XMLNode; 
    fread(buff,1,size,file);
    fclose(file);
    buff[size] = '\0';

    doc->root = XMLNodeNew(NULL);
    char lex[256];
    int lexi = 0;
    int i =0;
    XMLNode* currentNode = nullptr;
    while(buff[i] != '\0')
    {
        if(buff[i] == '<') //beginning of node
        {
            lex[lexi] = '\0';

	    /* setting innerText */
            if(lexi > 0)//if len is > 0 then there is some text
            {
                if(!currentNode) //if currnode is null then that text is outside tag
                {
                    std::cout << "Text outside of document\n";
                    return false;
                }
                currentNode->innerText = strdup(lex);
                lexi = 0;
            }

            /* setting end of node */
            if(buff[i+1] == '/')
            {
                i+=2;
                while(buff[i] != '>') //keep building lex untill hit >
                    lex[lexi++] = buff[i++];
                lex[lexi] = '\0';// put \0 to omplete the str

                if(!currentNode) //checking if its the start tag or end
                {
                    std::cerr << "Already at the root\n ";
                    return false; 
                }
                if(strcmp(currentNode->tag, lex))
                {
                    std::cerr << "Mismathced tags " << currentNode->tag << "!=" << lex; //currNode tag isnt same as whats in lex
                    return false;
                }
		
		//when done with currNode, move to its parent
                currentNode = currentNode->parent;
                i++;
                continue;
            }

	    /* setting currentNode */
            if(!currentNode) //set to parent/root when current node is null
                currentNode = doc->root;
            else   
                currentNode = XMLNodeNew(currentNode); //creating a newnode with cuuNode as its parent
            
            //getting beginning of tag (loop)
            i++;

            XMLAttribute currAttr = {0,0};
            while(buff[i] != '>')
            {
                lex[lexi++] = buff[i++];
            // lex[lexi] = '\0'; //completes 1st str inside tag
            // currentNode->tag = strdup(lex); //heap allocating tag's text inside node's tag field

                //tag name
                if(buff[i] == ' ' && !currentNode->tag)
                {
                    lex[lexi] = '\0';
                    currentNode->tag = strdup(lex);
                    lexi = 0;
                    i++;
                    continue;
                }

                //attribute key
                if(buff[i] == '=')
                {
                    lex[lexi] = '\0';
                    currAttr.key = strdup(lex);
                    lexi = 0;
                    continue;
                }

                //attribute value
                if(buff[i] == '"')
                {
                    if(!currAttr.key)
                    {
                        std::cerr << "Value has no key \n";
                        return false;
                    }

                    lexi =0;
                    i++;

                    while(buff[i] != '"')
                        lex[lexi++] = buff[i++];
                    lex[lexi] = '\0';
                    currAttr.value = strdup(lex);
                    XMLAttributeListAdd(&currentNode->attributes, &currAttr);
                    currAttr.key = nullptr;
                    currAttr.value = nullptr;
                    lexi = 0;
                    i++;
                    continue;

                }
            }

            /*resetting lexer */
            lexi =0;
            i++;
            continue;
        }
        else //keep appending from buf to lex
        {
            lex[lexi++] = buff[i++];
        }
    }

    return true;
}


void XMLDocumentFree(XMLDocument* doc)
{
    XMLNodeFree(doc->root);
}




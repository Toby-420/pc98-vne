#include <string.h>
#include <stdlib.h>

#include "story.h"
#include "textbox.h"

char *handle_choice(gfx_state_t *gfx, char *text)
{
    char *choice_token, *action_token;
    char *choices[MAX_CHOICES], *actions[MAX_CHOICES];
    int i, num_choices, chosen_option;


    // Tokenise choices
    for (i = 0; i < MAX_CHOICES; i++)
    {
        // Make sure first token is stored
        if (i == 0)
        {
            choice_token = strtok(text, "&");
        } else
        {
            choice_token = strtok(NULL, "&");
        }

        if (choice_token == NULL)
        {
            num_choices = i + 1;
            break;
        }

        choices[i] = malloc(strlen(choice_token) + 1);

        if (choices[i] == NULL) break;

        strcpy(choices[i], choice_token);

    }

    // Tokenise actions
    for (i = 0; i < num_choices; i++)
    {
        (void)strtok(choices[i], "|");

        action_token = strtok(NULL, "|");

        if (action_token == NULL) break;

        actions[i] = malloc(strlen(action_token) + 1);

        if (actions[i] == NULL) break;

        strcpy(actions[i], action_token);
    }

    chosen_option = menu_show(gfx, choices, num_choices - 1);

    if (chosen_option < 0 || chosen_option >= num_choices - 1)
    {
        // Free allocated memory before returning
        for (i = 0; i < num_choices; i++)
        {
            free(choices[i]);
            free(actions[i]);
        }
        return NULL;
    }

    // Copy the chosen label before freeing the other actions
    {
        char *label = malloc(strlen(actions[chosen_option]) + 1);

        if (label != NULL)
            strcpy(label, actions[chosen_option]);

        for (i = 0; i < num_choices; i++)
        {
            free(choices[i]);
            free(actions[i]);
        }

        return label;
    }
}

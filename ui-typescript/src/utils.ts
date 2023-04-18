export type Constructor<T> = new (...args: any[]) => T;
export const _document = typeof window === 'undefined' ? {} : document;

export function getRequiredElementById(id: string): HTMLElement {
    const el = document.getElementById(id);

    if (el !== undefined && el !== null)
        return el;

    throw new Error(`${id} should be input`);
}

export function getExactElementById<T extends HTMLElement>(id: string, filterType: Constructor<T>): T {
    const el = document.getElementById(id);

    if (el instanceof filterType)
        return el;

    throw new Error(`${id} should be type ${filterType}`);
}

export function getInputById(id: string): HTMLInputElement {
    const el = getExactElementById(id, HTMLInputElement);

    if (el instanceof HTMLInputElement)
        return el;

    throw new Error(`${id} should be input`);
}

export function getSelectById(id: string): HTMLSelectElement {
    const el = getExactElementById(id, HTMLSelectElement);

    if (el instanceof HTMLSelectElement)
        return el;

    throw new Error(`${id} should be input`);
}

export function getNumberValueById(id: string): number {
    const el = getInputById(id);

    return Number(el.value);
}

export function toggleClass(el: HTMLElement, className: string, shoudHaveClass: boolean): void {
    if (shoudHaveClass)
        el.classList.add(className);
    else
        el.classList.remove(className);
}

export function toggleVisibility(el: HTMLElement, visible: boolean): void {
    if (visible)
        el.style.display = '';
    else
        el.style.display = 'none';
}

export function renderVariableValue(variableName: string, value: string | number): void {
    const elements = document.querySelectorAll(`[data-var="${variableName}"]`);

    elements.forEach(x => x.textContent = value.toString());
}